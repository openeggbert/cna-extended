// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/SpatialHash3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;

    int SpatialHash3DCellKeyEXT::GetHashCode() const
    {
        return System::HashCode::Combine(X, Y, Z);
    }

    SpatialHash3DEXT::SpatialHash3DEXT(float cellSize) : cellSize_(cellSize)
    {
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(cellSize, "cellSize");
    }

    void SpatialHash3DEXT::Insert(ICollisionActor3DEXT* actor)
    {
        if (actorCells_.contains(actor))
        {
            return;
        }

        actors_.push_back(actor);
        InsertIntoCells(actor);
    }

    bool SpatialHash3DEXT::Remove(ICollisionActor3DEXT* actor)
    {
        const auto actorCellsIt = actorCells_.find(actor);
        if (actorCellsIt == actorCells_.end())
        {
            return false;
        }

        for (const SpatialHash3DCellKeyEXT& cell : actorCellsIt->second)
        {
            const auto cellIt = cells_.find(cell);
            if (cellIt != cells_.end())
            {
                std::vector<ICollisionActor3DEXT*>& cellActors = cellIt->second;
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

    std::vector<ICollisionActor3DEXT*> SpatialHash3DEXT::Query(const BoundingBox& bounds) const
    {
        std::vector<ICollisionActor3DEXT*> results;
        int minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;
        GetCellRange(bounds, minX, minY, minZ, maxX, maxY, maxZ);

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                for (int z = minZ; z <= maxZ; z++)
                {
                    const SpatialHash3DCellKeyEXT cell(x, y, z);
                    const auto cellIt = cells_.find(cell);
                    if (cellIt != cells_.end())
                    {
                        for (ICollisionActor3DEXT* actor : cellIt->second)
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
        }

        return results;
    }

    void SpatialHash3DEXT::Reset()
    {
        cells_.clear();
        actorCells_.clear();

        for (ICollisionActor3DEXT* actor : actors_)
        {
            InsertIntoCells(actor);
        }
    }

    void SpatialHash3DEXT::InsertIntoCells(ICollisionActor3DEXT* actor)
    {
        const BoundingBox actorBounds = actor->getShapeProperty().getBoundingBoxProperty();
        std::vector<SpatialHash3DCellKeyEXT> occupiedCells;
        int minX = 0, minY = 0, minZ = 0, maxX = 0, maxY = 0, maxZ = 0;
        GetCellRange(actorBounds, minX, minY, minZ, maxX, maxY, maxZ);

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                for (int z = minZ; z <= maxZ; z++)
                {
                    AddToCell(x, y, z, actor);
                    occupiedCells.emplace_back(x, y, z);
                }
            }
        }

        actorCells_[actor] = std::move(occupiedCells);
    }

    void SpatialHash3DEXT::AddToCell(int x, int y, int z, ICollisionActor3DEXT* actor)
    {
        const SpatialHash3DCellKeyEXT cell(x, y, z);
        cells_[cell].push_back(actor);
    }

    void SpatialHash3DEXT::GetCellRange(const BoundingBox& bounds, int& minX, int& minY, int& minZ, int& maxX, int& maxY, int& maxZ) const
    {
        minX = GetCellIndex(bounds.Min.X);
        minY = GetCellIndex(bounds.Min.Y);
        minZ = GetCellIndex(bounds.Min.Z);
        maxX = GetCellIndex(bounds.Max.X);
        maxY = GetCellIndex(bounds.Max.Y);
        maxZ = GetCellIndex(bounds.Max.Z);
    }

    int SpatialHash3DEXT::GetCellIndex(float value) const
    {
        return static_cast<int>(std::floor(value / cellSize_));
    }
}
