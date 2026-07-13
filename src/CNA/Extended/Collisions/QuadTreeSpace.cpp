// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Collisions/QuadTreeSpace.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/Collisions/QuadtreeData.hpp"

#include <algorithm>

namespace CNA::Extended::Collisions
{
    QuadTreeSpace::QuadTreeSpace(const BoundingBox2D& boundary) : collisionTree_(boundary)
    {
    }

    QuadTreeSpace::~QuadTreeSpace() = default;
    QuadTreeSpace::QuadTreeSpace(QuadTreeSpace&&) noexcept = default;
    QuadTreeSpace& QuadTreeSpace::operator=(QuadTreeSpace&&) noexcept = default;

    void QuadTreeSpace::Insert(ICollisionActor* actor)
    {
        if (!actorData_.contains(actor))
        {
            auto data = std::make_unique<QuadtreeData>(*actor);
            QuadtreeData* dataPtr = data.get();
            actorData_.emplace(actor, std::move(data));
            collisionTree_.Insert(*dataPtr);
            actors_.push_back(actor);
        }
    }

    bool QuadTreeSpace::Remove(ICollisionActor* actor)
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

    std::vector<ICollisionActor*> QuadTreeSpace::Query(const BoundingBox2D& bounds) const
    {
        std::vector<ICollisionActor*> results;
        for (QuadtreeData* data : collisionTree_.Query(bounds))
        {
            results.push_back(&data->getTargetProperty());
        }
        return results;
    }

    void QuadTreeSpace::Reset()
    {
        collisionTree_.ClearAll();

        for (const auto& [actor, data] : actorData_)
        {
            collisionTree_.Insert(*data);
        }

        collisionTree_.Shake();
    }
}
