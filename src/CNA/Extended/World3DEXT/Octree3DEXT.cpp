// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Octree3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OctreeNodeData3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;

    Octree3DEXT::Octree3DEXT(const BoundingBox& boundary) : collisionTree_(boundary)
    {
    }

    Octree3DEXT::~Octree3DEXT() = default;
    Octree3DEXT::Octree3DEXT(Octree3DEXT&&) noexcept = default;
    Octree3DEXT& Octree3DEXT::operator=(Octree3DEXT&&) noexcept = default;

    void Octree3DEXT::Insert(ICollisionActor3DEXT* actor)
    {
        if (!actorData_.contains(actor))
        {
            auto data = std::make_unique<OctreeNodeData3DEXT>(*actor);
            OctreeNodeData3DEXT* dataPtr = data.get();
            actorData_.emplace(actor, std::move(data));
            collisionTree_.Insert(*dataPtr);
            actors_.push_back(actor);
        }
    }

    bool Octree3DEXT::Remove(ICollisionActor3DEXT* actor)
    {
        const auto it = actorData_.find(actor);
        if (it != actorData_.end())
        {
            it->second->RemoveFromAllParents();
            actorData_.erase(it);
            collisionTree_.Shake();
            actors_.erase(std::remove(actors_.begin(), actors_.end(), actor), actors_.end());
            return true;
        }

        return false;
    }

    std::vector<ICollisionActor3DEXT*> Octree3DEXT::Query(const BoundingBox& bounds) const
    {
        std::vector<ICollisionActor3DEXT*> results;
        for (OctreeNodeData3DEXT* data : collisionTree_.Query(bounds))
        {
            results.push_back(&data->getTargetProperty());
        }
        return results;
    }

    void Octree3DEXT::Reset()
    {
        collisionTree_.ClearAll();

        for (const auto& [actor, data] : actorData_)
        {
            collisionTree_.Insert(*data);
        }

        collisionTree_.Shake();
    }
}
