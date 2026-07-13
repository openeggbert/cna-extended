// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ECS/World.hpp"

namespace CNA::Extended::ECS
{
    World::World()
    {
        auto componentManager = std::make_unique<ComponentManager>();
        auto entityManager = std::make_unique<EntityManager>(*componentManager);

        componentManager_ = componentManager.get();
        entityManager_ = entityManager.get();

        RegisterSystem(std::move(componentManager));
        RegisterSystem(std::move(entityManager));

        entityAddedToken_ = entityManager_->EntityAdded.Add([this](int entityId) { EntityAdded(entityId); });
        entityRemovedToken_ = entityManager_->EntityRemoved.Add([this](int entityId) { EntityRemoved(entityId); });
        entityChangedToken_ = entityManager_->EntityChanged.Add([this](int entityId) { EntityChanged(entityId); });
    }

    void World::RegisterSystem(std::unique_ptr<Systems::ISystem> system)
    {
        Systems::ISystem* raw = system.get();

        if (auto* updateSystem = dynamic_cast<Systems::IUpdateSystem*>(raw))
        {
            updateSystems_.Add(updateSystem);
        }

        if (auto* drawSystem = dynamic_cast<Systems::IDrawSystem*>(raw))
        {
            drawSystems_.Add(drawSystem);
        }

        raw->Initialize(*this);
        systems_.push_back(std::move(system));
    }

    void World::Update(GameTime& gameTime)
    {
        for (Systems::IUpdateSystem* system : updateSystems_)
        {
            system->Update(gameTime);
        }
    }

    void World::Draw(const GameTime& gameTime)
    {
        for (Systems::IDrawSystem* system : drawSystems_)
        {
            system->Draw(gameTime);
        }
    }

    void World::Dispose()
    {
        entityManager_->EntityAdded.Remove(entityAddedToken_);
        entityManager_->EntityRemoved.Remove(entityRemovedToken_);
        entityManager_->EntityChanged.Remove(entityChangedToken_);

        for (Systems::IUpdateSystem* system : updateSystems_)
        {
            system->Dispose();
        }

        for (Systems::IDrawSystem* system : drawSystems_)
        {
            system->Dispose();
        }

        updateSystems_.Clear();
        drawSystems_.Clear();

        SimpleDrawableGameComponent::Dispose();
    }

    const std::string& World::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.ECS.World";
        return typeName;
    }
}
