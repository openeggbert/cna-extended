// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/SpatialHash.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/CollisionShape2D.hpp"
#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::Collisions
{
    int SpatialHashCellKey::GetHashCode() const
    {
        return System::HashCode::Combine(X, Y);
    }

    SpatialHash::SpatialHash(const SizeF& size) : cellSize_(size)
    {
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(size.Width, "size.Width");
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(size.Height, "size.Height");
    }

    void SpatialHash::Insert(ICollisionActor* actor)
    {
        if (actorCells_.contains(actor))
        {
            return;
        }

        actors_.push_back(actor);
        InsertIntoCells(actor);
    }

    bool SpatialHash::Remove(ICollisionActor* actor)
    {
        const auto actorCellsIt = actorCells_.find(actor);
        if (actorCellsIt == actorCells_.end())
        {
            return false;
        }

        for (const SpatialHashCellKey& cell : actorCellsIt->second)
        {
            const auto cellIt = cells_.find(cell);
            if (cellIt != cells_.end())
            {
                std::vector<ICollisionActor*>& cellActors = cellIt->second;
                cellActors.erase(std::remove(cellActors.begin(), cellActors.end(), actor), cellActors.end());

                if (cellActors.empty())
                {
                    cells_.erase(cellIt);
                }
            }
        }

        actorCells_.erase(actorCellsIt);
        actors_.erase(std::remove(actors_.begin(), actors_.end(), actor), actors_.end());
        return true;
    }

    std::vector<ICollisionActor*> SpatialHash::Query(const BoundingBox2D& bounds) const
    {
        std::vector<ICollisionActor*> results;
        int minX = 0;
        int minY = 0;
        int maxX = 0;
        int maxY = 0;
        GetCellRange(bounds, minX, minY, maxX, maxY);

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                const SpatialHashCellKey cell(x, y);
                const auto cellIt = cells_.find(cell);
                if (cellIt != cells_.end())
                {
                    for (ICollisionActor* actor : cellIt->second)
                    {
                        if (bounds.Intersects(actor->getShapeProperty().getBoundingBoxProperty())
                            && std::find(results.begin(), results.end(), actor) == results.end())
                        {
                            results.push_back(actor);
                        }
                    }
                }
            }
        }

        return results;
    }

    void SpatialHash::Reset()
    {
        cells_.clear();
        actorCells_.clear();

        for (ICollisionActor* actor : actors_)
        {
            InsertIntoCells(actor);
        }
    }

    void SpatialHash::InsertIntoCells(ICollisionActor* actor)
    {
        const BoundingBox2D actorBounds = actor->getShapeProperty().getBoundingBoxProperty();
        std::vector<SpatialHashCellKey> occupiedCells;
        int minX = 0;
        int minY = 0;
        int maxX = 0;
        int maxY = 0;
        GetCellRange(actorBounds, minX, minY, maxX, maxY);

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                AddToCell(x, y, actor);
                occupiedCells.emplace_back(x, y);
            }
        }

        actorCells_[actor] = std::move(occupiedCells);
    }

    void SpatialHash::AddToCell(int x, int y, ICollisionActor* actor)
    {
        const SpatialHashCellKey cell(x, y);
        cells_[cell].push_back(actor);
    }

    void SpatialHash::GetCellRange(const BoundingBox2D& bounds, int& minX, int& minY, int& maxX, int& maxY) const
    {
        minX = GetCellIndex(bounds.Min.X, cellSize_.Width);
        minY = GetCellIndex(bounds.Min.Y, cellSize_.Height);
        maxX = GetCellIndex(bounds.Max.X, cellSize_.Width);
        maxY = GetCellIndex(bounds.Max.Y, cellSize_.Height);
    }

    int SpatialHash::GetCellIndex(float value, float cellSize)
    {
        return static_cast<int>(std::floor(value / cellSize));
    }
}
