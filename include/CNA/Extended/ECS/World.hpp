// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/World.cs.
//
// Ownership: upstream's `WorldBuilder.Build()` constructs each registered `ISystem` beforehand and
// hands it to `World.RegisterSystem`; after `Build()` returns, `World`'s own `_updateSystems`/
// `_drawSystems` bags are the only remaining GC roots keeping those systems alive. This project
// makes that ownership transfer explicit: `RegisterSystem` takes `std::unique_ptr<ISystem>` and
// stores it in `systems_` (the true owning collection); the update/draw bags
// (`updateSystems_`/`drawSystems_`) hold non-owning raw pointers into it, obtained via
// `dynamic_cast` -- a direct translation of upstream's own `system is IUpdateSystem`/
// `system is IDrawSystem` runtime pattern-match checks (a system can independently implement
// either, both, or neither interface; `EntitySystem` alone implements neither -- only
// `EntityUpdateSystem`/`EntityDrawSystem` and the base `UpdateSystem`/`DrawSystem` do).
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "CNA/Extended/ECS/ComponentManager.hpp"
#include "CNA/Extended/ECS/EntityManager.hpp"
#include "CNA/Extended/ECS/Systems/DrawSystem.hpp"
#include "CNA/Extended/ECS/Systems/ISystem.hpp"
#include "CNA/Extended/ECS/Systems/UpdateSystem.hpp"
#include "CNA/Extended/SimpleDrawableGameComponent.hpp"
#include "System/MulticastAction.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::ECS
{
    /**
     * @brief The central ECS container: owns EntityManager/ComponentManager, registers and runs
     * Systems each Update/Draw.
     * @see WorldBuilder, the normal way to construct a World with its systems already registered
     * (prefer it over this class's own constructor -- see WorldBuilder's own doc comment).
     * @see Entity, created via CreateEntity() and used to attach/query components.
     */
    class World : public SimpleDrawableGameComponent
    {
    public:
        /** @brief Constructs an empty World. Prefer WorldBuilder::Build(), which also registers your systems. */
        World();

        /** @brief Gets the EntityManager backing this World. Upstream marks this `internal`; kept public per this port's established convention. */
        [[nodiscard]] EntityManager& getEntityManagerProperty() { return *entityManager_; }
        /** @brief Gets the ComponentManager backing this World. Upstream marks this `internal`; kept public per this port's established convention. */
        [[nodiscard]] ComponentManager& getComponentManagerProperty() { return *componentManager_; }

        /** @brief Raised (with the entity ID) after an entity is added, during the next Update cycle. */
        System::MulticastAction<int> EntityAdded;
        /** @brief Raised (with the entity ID) before a destroyed entity's components are removed, during the next Update cycle. */
        System::MulticastAction<int> EntityRemoved;
        /** @brief Raised (with the entity ID) whenever an entity's component composition changes, during the next Update cycle. */
        System::MulticastAction<int> EntityChanged;

        /** @brief Gets the number of currently-active entities. */
        [[nodiscard]] int getEntityCountProperty() const { return entityManager_->getActiveCountProperty(); }

        /** @brief Registers @p system, taking ownership of it, and initializes it. */
        void RegisterSystem(std::unique_ptr<Systems::ISystem> system);

        /** @brief Retrieves entity @p entityId, or nullptr if it has been destroyed or the ID is invalid. */
        [[nodiscard]] Entity* GetEntity(int entityId) { return entityManager_->Get(entityId); }

        /** @brief Creates a new entity, usable immediately to attach components. EntityAdded is not raised until the next Update. */
        Entity& CreateEntity() { return entityManager_->Create(); }

        /** @brief Queues entity @p entityId for destruction on the next Update. */
        void DestroyEntity(int entityId) { entityManager_->Destroy(entityId); }
        /** @brief Queues @p entity for destruction on the next Update. */
        void DestroyEntity(const Entity& entity) { entityManager_->Destroy(entity); }

        // SimpleGameComponent::Update takes GameTime by mutable reference (CNA's own IUpdateable
        // convention); SimpleDrawableGameComponent::Draw takes it by const reference. Both
        // signatures below match their respective base class exactly, not each other.
        void Update(GameTime& gameTime) override;
        void Draw(const GameTime& gameTime) override;
        void Dispose() override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        // Sole owners: componentManager_/entityManager_ below are non-owning raw pointers into
        // the ComponentManager/EntityManager instances created in the constructor and moved into
        // systems_ via RegisterSystem -- systems_ is their one true owner (matching upstream:
        // ComponentManager/EntityManager are single GC-tracked objects referenced from both the
        // ComponentManager/EntityManager properties AND the update-systems bag; C++ needs exactly
        // one owner, so it's systems_, and everywhere else holds a non-owning pointer into it).
        std::vector<std::unique_ptr<Systems::ISystem>> systems_;
        Collections::Bag<Systems::IUpdateSystem*> updateSystems_;
        Collections::Bag<Systems::IDrawSystem*> drawSystems_;

        ComponentManager* componentManager_ = nullptr;
        EntityManager* entityManager_ = nullptr;

        System::MulticastAction<int>::Token entityAddedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token entityRemovedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token entityChangedToken_ = System::MulticastAction<int>::InvalidToken;
    };
}
