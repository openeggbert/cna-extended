// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/WorldManagerTests.cs.
// `DummySystem : EntitySystem, IUpdateSystem, IDrawSystem` (three bases sharing ISystem) exercises
// exactly the virtual-inheritance diamond documented in Systems/UpdateSystem.hpp's header comment.
// `entity.Attach(new Transform2())`-style upstream heap allocations are ported using stack-local
// components (see ComponentMapperTests.cpp's header comment for why).
#include "CNA/Extended/ECS/World.hpp"

#include "CNA/Extended/ECS/Systems/EntitySystem.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/Transform.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::ECS
{
    using Microsoft::Xna::Framework::GameTime;
    using System::TimeSpan;

    namespace
    {
        class TestComponent
        {
        };

        class DummyComponent
        {
        };

        class DummySystem : public Systems::EntitySystem, public Systems::IUpdateSystem, public Systems::IDrawSystem
        {
        public:
            DummySystem() : EntitySystem(AspectBuilder().All({std::type_index(typeid(DummyComponent))})) {}

            std::vector<int> AddedEntitiesId;
            std::vector<int> RemovedEntitiesId;

            // Deliberately does not initialize any ComponentMapper -- matches upstream's own
            // regression test for https://github.com/craftworkgames/MonoGame.Extended/issues/707.
            void Initialize(ComponentManager& componentManager) override { (void)componentManager; }

            void Draw(const GameTime& gameTime) override { (void)gameTime; }
            void Update(const GameTime& gameTime) override { (void)gameTime; }

        protected:
            void OnEntityAdded(int entityId) override
            {
                EntitySystem::OnEntityAdded(entityId);
                AddedEntitiesId.push_back(entityId);
            }

            void OnEntityRemoved(int entityId) override
            {
                EntitySystem::OnEntityRemoved(entityId);
                RemovedEntitiesId.push_back(entityId);
            }
        };
    }

    TEST(WorldManagerTests, CrudEntity)
    {
        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));

        auto dummySystemOwner = std::make_unique<DummySystem>();
        DummySystem* dummySystem = dummySystemOwner.get();

        WorldBuilder worldBuilder;
        worldBuilder.AddSystem(std::move(dummySystemOwner));
        std::unique_ptr<World> world = worldBuilder.Build();

        std::vector<int> addedEntities;
        std::vector<int> removedEntities;
        std::vector<int> changedEntities;

        world->Initialize();

        world->EntityAdded += [&](int entityId) { addedEntities.push_back(entityId); };
        Entity& entity = world->CreateEntity();
        Transform2 transform;
        entity.Attach(&transform);
        world->Update(gameTime);
        world->Draw(gameTime);
        Entity* otherEntity = world->GetEntity(entity.getIdProperty());
        ASSERT_NE(otherEntity, nullptr);
        EXPECT_EQ(*otherEntity, entity);
        EXPECT_TRUE(otherEntity->Has<Transform2>());
        EXPECT_NE(std::find(dummySystem->AddedEntitiesId.begin(), dummySystem->AddedEntitiesId.end(), entity.getIdProperty()),
            dummySystem->AddedEntitiesId.end());
        ASSERT_EQ(addedEntities.size(), 1u);
        EXPECT_EQ(addedEntities[0], entity.getIdProperty());

        world->EntityChanged += [&](int entityId) { changedEntities.push_back(entityId); };
        TestComponent testComponent;
        entity.Attach(&testComponent);
        world->Update(gameTime);
        ASSERT_EQ(changedEntities.size(), 1u);
        EXPECT_EQ(changedEntities[0], entity.getIdProperty());

        world->EntityRemoved += [&](int entityId) { removedEntities.push_back(entityId); };
        const int entityId = entity.getIdProperty();
        entity.Destroy();
        world->Update(gameTime);
        world->Draw(gameTime);
        otherEntity = world->GetEntity(entityId);
        EXPECT_EQ(otherEntity, nullptr);
        EXPECT_NE(std::find(dummySystem->RemovedEntitiesId.begin(), dummySystem->RemovedEntitiesId.end(), entityId),
            dummySystem->RemovedEntitiesId.end());
        ASSERT_EQ(removedEntities.size(), 1u);
        EXPECT_EQ(removedEntities[0], entityId);
    }
}
