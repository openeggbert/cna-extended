// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/OctreeEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/HashCode.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;

    int OctreeCellKeyEXT::GetHashCode() const
    {
        return System::HashCode::Combine(X, Y, Z);
    }

    OctreeEXT::OctreeEXT(float cellSize) : cellSize_(cellSize)
    {
        System::ArgumentOutOfRangeException::ThrowIfNegativeOrZero(cellSize, "cellSize");
    }

    void OctreeEXT::Insert(ICollisionActor3DEXT* actor)
    {
        if (actorCells_.contains(actor))
        {
            return;
        }

        actors_.push_back(actor);
        InsertIntoCells(actor);
    }

    bool OctreeEXT::Remove(ICollisionActor3DEXT* actor)
    {
        const auto actorCellsIt = actorCells_.find(actor);
        if (actorCellsIt == actorCells_.end())
        {
            return false;
        }

        for (const OctreeCellKeyEXT& cell : actorCellsIt->second)
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

    std::vector<ICollisionActor3DEXT*> OctreeEXT::Query(const BoundingBox& bounds) const
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
                    const OctreeCellKeyEXT cell(x, y, z);
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

    void OctreeEXT::Reset()
    {
        cells_.clear();
        actorCells_.clear();

        for (ICollisionActor3DEXT* actor : actors_)
        {
            InsertIntoCells(actor);
        }
    }

    void OctreeEXT::InsertIntoCells(ICollisionActor3DEXT* actor)
    {
        const BoundingBox actorBounds = actor->getShapeProperty().getBoundingBoxProperty();
        std::vector<OctreeCellKeyEXT> occupiedCells;
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

    void OctreeEXT::AddToCell(int x, int y, int z, ICollisionActor3DEXT* actor)
    {
        const OctreeCellKeyEXT cell(x, y, z);
        cells_[cell].push_back(actor);
    }

    void OctreeEXT::GetCellRange(const BoundingBox& bounds, int& minX, int& minY, int& minZ, int& maxX, int& maxY, int& maxZ) const
    {
        minX = GetCellIndex(bounds.Min.X);
        minY = GetCellIndex(bounds.Min.Y);
        minZ = GetCellIndex(bounds.Min.Z);
        maxX = GetCellIndex(bounds.Max.X);
        maxY = GetCellIndex(bounds.Max.Y);
        maxZ = GetCellIndex(bounds.Max.Z);
    }

    int OctreeEXT::GetCellIndex(float value) const
    {
        return static_cast<int>(std::floor(value / cellSize_));
    }
}
