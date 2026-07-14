// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ActorPairKey3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SpatialHash3DEXT.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;

    CollisionWorld3DEXT::CollisionWorld3DEXT() : broadphase_(std::make_unique<SpatialHash3DEXT>(16.0f))
    {
    }

    CollisionWorld3DEXT::CollisionWorld3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> broadphase) : broadphase_(std::move(broadphase))
    {
        if (broadphase_ == nullptr)
        {
            throw std::invalid_argument("broadphase must not be null.");
        }
    }

    CollisionWorld3DEXT::~CollisionWorld3DEXT() = default;

    void CollisionWorld3DEXT::Insert(ICollisionActor3DEXT* actor)
    {
        broadphase_->Insert(actor);
    }

    bool CollisionWorld3DEXT::Remove(ICollisionActor3DEXT* actor)
    {
        return broadphase_->Remove(actor);
    }

    bool CollisionWorld3DEXT::Contains(ICollisionActor3DEXT* actor) const
    {
        const std::vector<ICollisionActor3DEXT*>& actors = broadphase_->GetActors();
        return std::find(actors.begin(), actors.end(), actor) != actors.end();
    }

    void CollisionWorld3DEXT::Rebuild()
    {
        broadphase_->Reset();
    }

    std::vector<ICollisionActor3DEXT*> CollisionWorld3DEXT::QueryCandidates(const BoundingBox& bounds) const
    {
        return broadphase_->Query(bounds);
    }

    std::vector<ICollisionActor3DEXT*> CollisionWorld3DEXT::QueryCandidates(ICollisionActor3DEXT* actor) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        return broadphase_->Query(actor->getShapeProperty().getBoundingBoxProperty());
    }

    std::vector<CollisionEvent3DEXT> CollisionWorld3DEXT::QueryCollisions(ICollisionActor3DEXT* actor) const
    {
        if (actor == nullptr)
        {
            throw std::invalid_argument("actor must not be null.");
        }

        std::vector<CollisionEvent3DEXT> results;
        for (ICollisionActor3DEXT* candidate : QueryCandidates(actor))
        {
            if (actor == candidate)
            {
                continue;
            }

            CollisionResult3DEXT result;
            if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
            {
                continue;
            }

            results.emplace_back(*candidate, result);
        }
        return results;
    }

    std::vector<CollisionPair3DEXT> CollisionWorld3DEXT::QueryCollisionPairs() const
    {
        std::vector<CollisionPair3DEXT> results;
        std::unordered_set<ActorPairKey3DEXT> processedPairs;

        for (ICollisionActor3DEXT* actor : broadphase_->GetActors())
        {
            for (ICollisionActor3DEXT* candidate : broadphase_->Query(actor->getShapeProperty().getBoundingBoxProperty()))
            {
                if (actor == candidate)
                {
                    continue;
                }

                CollisionResult3DEXT result;
                if (!actor->getShapeProperty().TryGetCollision(candidate->getShapeProperty(), result))
                {
                    continue;
                }

                ActorPairKey3DEXT actorPair(*actor, *candidate);
                if (!processedPairs.insert(actorPair).second)
                {
                    continue;
                }

                results.emplace_back(*actor, *candidate, result);
            }
        }
        return results;
    }
}
