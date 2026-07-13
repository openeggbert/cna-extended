// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/EntitySubscription.cs (upstream `internal`, kept public per
// this project's established internal-visibility convention). `event -= handler` (identity-based
// unsubscription) has no direct C++ equivalent for lambda subscribers; translated using
// `System::MulticastAction<int>`'s token-based `Add`/`Remove` (see that class's own header
// comment for why). Preserved, not "fixed": upstream's `Dispose()` unsubscribes `EntityAdded`/
// `EntityRemoved` but NOT `EntityChanged` -- a genuine upstream asymmetry (its `OnEntityChanged`
// handler stays subscribed for the EntityManager's remaining lifetime even after this
// EntitySubscription is disposed). Reproduced exactly: only two of the three tokens are removed
// in Dispose() here too.
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "CNA/Extended/ECS/Aspect.hpp"
#include "System/IDisposable.hpp"
#include "System/MulticastAction.hpp"

namespace CNA::Extended::ECS
{
    class EntityManager;

    /** @brief Tracks which entities currently satisfy an Aspect, rebuilding lazily when the entity set changes. */
    class EntitySubscription : public System::IDisposable
    {
    public:
        EntitySubscription(EntityManager& entityManager, const Aspect& aspect);

        void Dispose() override;

        /** @brief Gets the IDs of entities currently satisfying this subscription's Aspect, rebuilding first if the entity set has changed. */
        [[nodiscard]] const Collections::Bag<int>& getActiveEntitiesProperty();

    private:
        void OnEntityAdded(int entityId);
        void OnEntityRemoved(int entityId);
        void OnEntityChanged(int entityId);
        void RebuildActives();

        EntityManager* entityManager_;
        Aspect aspect_;
        Collections::Bag<int> activeEntities_;
        bool rebuildActives_ = true;
        System::MulticastAction<int>::Token addedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token removedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token changedToken_ = System::MulticastAction<int>::InvalidToken;
    };
}
