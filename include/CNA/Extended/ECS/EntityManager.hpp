// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/EntityManager.cs.
//
// Ownership: upstream pools and reuses `Entity` objects (`Pool<Entity>`), relying on the GC to
// keep every Entity ever created alive for as long as anything (the pool's free list, or the
// active `_entityBag` slot) references it -- it never explicitly frees one. This project's
// `Collections::Pool<T>` (already ported) mirrors that recycling behavior exactly but, being a
// pure C++ pool with no GC backing it, never deletes an obtained/freed item either -- ownership of
// the actual memory has to live somewhere for the whole EntityManager's lifetime. `allEntities_`
// below is that somewhere: every `Entity` ever created is heap-allocated once by the pool's
// factory and owned there for pointer stability; `entityPool_`/`entityBag_` (matching upstream's
// `_entityPool`/`_entityBag`) hold non-owning `Entity*` into it, exactly mirroring upstream's own
// "one canonical Entity object per ID, reused via the pool" design -- the only change is *where*
// the canonical ownership lives (GC root vs. an explicit owning container), not the observable
// recycling behavior itself.
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "CNA/Extended/Collections/Pool.hpp"
#include "CNA/Extended/ECS/ComponentBits.hpp"
#include "CNA/Extended/ECS/Entity.hpp"
#include "CNA/Extended/ECS/Systems/UpdateSystem.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/MulticastAction.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::ECS
{
    using Microsoft::Xna::Framework::GameTime;

    class ComponentManager;

    /** @brief Owns entity ID assignment/reuse, tracks each entity's ComponentBits, and raises Added/Removed/Changed events once per Update. */
    class EntityManager : public Systems::UpdateSystem
    {
    public:
        static constexpr std::size_t DefaultBagSize = 128;

        explicit EntityManager(ComponentManager& componentManager);

        /** @brief Gets the total backing capacity for entity IDs. */
        [[nodiscard]] std::size_t getCapacityProperty() const { return entityBag_.getCapacityProperty(); }

        /** @brief Gets the IDs of every currently-active entity. */
        [[nodiscard]] std::vector<int> getEntitiesProperty() const;

        /** @brief Gets the number of currently-active entities. */
        [[nodiscard]] int getActiveCountProperty() const { return activeCount_; }

        /** @brief Raised (with the entity ID) once an added entity is fully initialized, during the next Update. */
        System::MulticastAction<int> EntityAdded;
        /** @brief Raised (with the entity ID) just before a destroyed entity's components are removed, during the next Update. */
        System::MulticastAction<int> EntityRemoved;
        /** @brief Raised (with the entity ID) when an entity's component composition changes, during the next Update. */
        System::MulticastAction<int> EntityChanged;

        /** @brief Creates a new entity immediately (usable right away), though EntityAdded isn't raised until the next Update. */
        Entity& Create();

        /** @brief Queues entity @p entityId for destruction on the next Update. Calling this more than once for the same ID has no additional effect. */
        void Destroy(int entityId);
        /** @brief Queues @p entity for destruction on the next Update. */
        void Destroy(const Entity& entity);

        /** @brief Gets the entity with @p entityId, or nullptr if it has been destroyed or the ID is invalid. */
        [[nodiscard]] Entity* Get(int entityId) const;

        /** @brief Gets the ComponentBits currently associated with @p entityId. */
        [[nodiscard]] ComponentBits GetComponentBits(int entityId) const;

        void Update(const GameTime& gameTime) override;

    private:
        void OnComponentsChanged(int entityId);

        ComponentManager* componentManager_;
        int nextId_ = 0;
        int activeCount_ = 0;

        std::vector<std::unique_ptr<Entity>> allEntities_;
        Collections::Pool<Entity> entityPool_;
        Collections::Bag<Entity*> entityBag_;
        Collections::Bag<int> addedEntities_;
        Collections::Bag<int> removedEntities_;
        Collections::Bag<int> changedEntities_;
        Collections::Bag<ComponentBits> entityToComponentBits_;
    };
}
