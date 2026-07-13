// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/Systems/EntitySystem.hpp"

#include "CNA/Extended/ECS/EntitySubscription.hpp"
#include "CNA/Extended/ECS/World.hpp"

namespace CNA::Extended::ECS::Systems
{
    EntitySystem::EntitySystem(AspectBuilder aspectBuilder) : aspectBuilder_(std::move(aspectBuilder)) {}

    EntitySystem::~EntitySystem() = default;

    void EntitySystem::Dispose()
    {
        // EntityChanged is deliberately not unsubscribed here -- matches upstream exactly (see
        // EntitySubscription.hpp's header comment for the same, upstream-original asymmetry).
        if (world_ != nullptr)
        {
            world_->getEntityManagerProperty().EntityAdded.Remove(addedToken_);
            world_->getEntityManagerProperty().EntityRemoved.Remove(removedToken_);
        }
    }

    const Collections::Bag<int>& EntitySystem::getActiveEntitiesProperty() const
    {
        return subscription_->getActiveEntitiesProperty();
    }

    void EntitySystem::Initialize(World& world)
    {
        world_ = &world;

        Aspect aspect = aspectBuilder_.Build(world.getComponentManagerProperty());
        subscription_ = std::make_unique<EntitySubscription>(world.getEntityManagerProperty(), aspect);
        addedToken_ = world_->getEntityManagerProperty().EntityAdded.Add([this](int entityId) { OnEntityAdded(entityId); });
        removedToken_ = world_->getEntityManagerProperty().EntityRemoved.Add([this](int entityId) { OnEntityRemoved(entityId); });
        changedToken_ = world_->getEntityManagerProperty().EntityChanged.Add([this](int entityId) { OnEntityChanged(entityId); });

        Initialize(world.getComponentManagerProperty());
    }

    void EntitySystem::DestroyEntity(int entityId)
    {
        world_->DestroyEntity(entityId);
    }

    Entity& EntitySystem::CreateEntity()
    {
        return world_->CreateEntity();
    }

    Entity* EntitySystem::GetEntity(int entityId)
    {
        return world_->GetEntity(entityId);
    }
}
