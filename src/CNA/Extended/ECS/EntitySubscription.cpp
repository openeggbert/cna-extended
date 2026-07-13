// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/EntitySubscription.hpp"

#include "CNA/Extended/ECS/EntityManager.hpp"

namespace CNA::Extended::ECS
{
    EntitySubscription::EntitySubscription(EntityManager& entityManager, const Aspect& aspect)
        : entityManager_(&entityManager), aspect_(aspect), activeEntities_(entityManager.getCapacityProperty())
    {
        addedToken_ = entityManager_->EntityAdded.Add([this](int entityId) { OnEntityAdded(entityId); });
        removedToken_ = entityManager_->EntityRemoved.Add([this](int entityId) { OnEntityRemoved(entityId); });
        changedToken_ = entityManager_->EntityChanged.Add([this](int entityId) { OnEntityChanged(entityId); });
    }

    void EntitySubscription::Dispose()
    {
        entityManager_->EntityAdded.Remove(addedToken_);
        entityManager_->EntityRemoved.Remove(removedToken_);
        // EntityChanged is deliberately not unsubscribed here -- see this file's header comment.
    }

    void EntitySubscription::OnEntityAdded(int entityId)
    {
        if (aspect_.IsInterested(entityManager_->GetComponentBits(entityId)))
        {
            activeEntities_.Add(entityId);
        }
    }

    void EntitySubscription::OnEntityRemoved(int entityId)
    {
        (void)entityId;
        rebuildActives_ = true;
    }

    void EntitySubscription::OnEntityChanged(int entityId)
    {
        (void)entityId;
        rebuildActives_ = true;
    }

    const Collections::Bag<int>& EntitySubscription::getActiveEntitiesProperty()
    {
        if (rebuildActives_)
        {
            RebuildActives();
        }
        return activeEntities_;
    }

    void EntitySubscription::RebuildActives()
    {
        activeEntities_.Clear();

        for (int entityId : entityManager_->getEntitiesProperty())
        {
            OnEntityAdded(entityId);
        }

        rebuildActives_ = false;
    }
}
