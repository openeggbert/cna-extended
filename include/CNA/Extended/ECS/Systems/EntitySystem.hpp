// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/Systems/EntitySystem.cs. Upstream's abstract
// `Initialize(IComponentMapperService mapperService)` takes an interface with a generic method
// (`ComponentMapper<T> GetMapper<T>()`); C++ cannot express a virtual member function template, so
// that interface is not separately materialized here -- `ComponentManager` (the only implementer
// anywhere in this codebase) is used directly instead. See ComponentManager.hpp's own header
// comment for the full reasoning, flagged there for the orchestrating session's awareness.
#pragma once

#include "CNA/Extended/Collections/Bag.hpp"
#include "CNA/Extended/ECS/AspectBuilder.hpp"
#include "CNA/Extended/ECS/Systems/ISystem.hpp"
#include "System/MulticastAction.hpp"

#include <memory>

namespace CNA::Extended::ECS
{
    class World;
    class ComponentManager;
    class EntitySubscription;
    class Entity;
}

namespace CNA::Extended::ECS::Systems
{
    /** @brief Base for a System that operates on the set of entities matching an AspectBuilder's requirements. See UpdateSystem.hpp's header comment for why ISystem is inherited virtually here. */
    class EntitySystem : public virtual ISystem
    {
    public:
        explicit EntitySystem(AspectBuilder aspectBuilder);
        ~EntitySystem() override;

        void Dispose() override;

        /** @brief Gets the IDs of entities currently matching this system's Aspect. */
        [[nodiscard]] const Collections::Bag<int>& getActiveEntitiesProperty() const;

        void Initialize(World& world) override;

        /** @brief Called once during Initialize, after this system's EntitySubscription is ready. Override to cache ComponentMapper<T>s via @p componentManager. */
        virtual void Initialize(ComponentManager& componentManager) = 0;

    protected:
        virtual void OnEntityChanged(int entityId) { (void)entityId; }
        virtual void OnEntityAdded(int entityId) { (void)entityId; }
        virtual void OnEntityRemoved(int entityId) { (void)entityId; }

        void DestroyEntity(int entityId);
        Entity& CreateEntity();
        Entity* GetEntity(int entityId);

    private:
        AspectBuilder aspectBuilder_;
        std::unique_ptr<EntitySubscription> subscription_;
        World* world_ = nullptr;
        System::MulticastAction<int>::Token addedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token removedToken_ = System::MulticastAction<int>::InvalidToken;
        System::MulticastAction<int>::Token changedToken_ = System::MulticastAction<int>::InvalidToken;
    };
}
