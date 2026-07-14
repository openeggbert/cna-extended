// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for TransformHierarchySystemEXT (see 3d.md/plan3d.md) -- there
// is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/TransformHierarchySystemEXT.hpp"

#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/World3DEXT/Transform3ComponentEXT.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::World3DEXT
{
    using ECS::World;
    using ECS::WorldBuilder;
    using Microsoft::Xna::Framework::GameTime;
    using System::TimeSpan;

    TEST(TransformHierarchySystemEXTTests, ThreeLevelChain_PropagatesWorldPositionThroughHierarchy)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<TransformHierarchySystemEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& grandparent = world->CreateEntity();
        ECS::Entity& parent = world->CreateEntity();
        ECS::Entity& child = world->CreateEntity();

        Transform3ComponentEXT grandparentTransform;
        grandparentTransform.TransformEXT.setPositionProperty(Vector3(10.0f, 0.0f, 0.0f));
        grandparent.Attach(&grandparentTransform);

        Transform3ComponentEXT parentTransform;
        parentTransform.TransformEXT.setPositionProperty(Vector3(5.0f, 0.0f, 0.0f));
        parentTransform.ParentEntityIdEXT = grandparent.getIdProperty();
        parent.Attach(&parentTransform);

        Transform3ComponentEXT childTransform;
        childTransform.TransformEXT.setPositionProperty(Vector3(2.0f, 0.0f, 0.0f));
        childTransform.ParentEntityIdEXT = parent.getIdProperty();
        child.Attach(&childTransform);

        // EntityManager (auto-registered by World's own constructor, before any
        // WorldBuilder-added system) processes pending entity additions before this
        // system's own Update() runs in the same World::Update() call -- see World.cpp's
        // constructor (RegisterSystem(componentManager) / RegisterSystem(entityManager)
        // happen before WorldBuilder registers user systems), so one Update() call is
        // enough to both register these three new entities with this system's Aspect and
        // resolve their hierarchy in the same frame.
        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        EXPECT_EQ(grandparentTransform.TransformEXT.getWorldPositionProperty(), Vector3(10.0f, 0.0f, 0.0f));
        EXPECT_EQ(parentTransform.TransformEXT.getWorldPositionProperty(), Vector3(15.0f, 0.0f, 0.0f));
        EXPECT_EQ(childTransform.TransformEXT.getWorldPositionProperty(), Vector3(17.0f, 0.0f, 0.0f));
    }

    TEST(TransformHierarchySystemEXTTests, MovingParent_PropagatesToChildImmediately)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<TransformHierarchySystemEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& parent = world->CreateEntity();
        ECS::Entity& child = world->CreateEntity();

        Transform3ComponentEXT parentTransform;
        parent.Attach(&parentTransform);

        Transform3ComponentEXT childTransform;
        childTransform.TransformEXT.setPositionProperty(Vector3(1.0f, 0.0f, 0.0f));
        childTransform.ParentEntityIdEXT = parent.getIdProperty();
        child.Attach(&childTransform);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);
        EXPECT_EQ(childTransform.TransformEXT.getWorldPositionProperty(), Vector3(1.0f, 0.0f, 0.0f));

        // Moving the parent after the hierarchy is wired doesn't need another
        // world->Update() to reach the child: Transform3's own TransformBecameDirty
        // propagation (already ported, unmodified here) already handles that live, the
        // same way it does for any other Transform3 parent/child pair.
        parentTransform.TransformEXT.setPositionProperty(Vector3(100.0f, 0.0f, 0.0f));
        EXPECT_EQ(childTransform.TransformEXT.getWorldPositionProperty(), Vector3(101.0f, 0.0f, 0.0f));
    }

    TEST(TransformHierarchySystemEXTTests, DetachingParent_ChildFallsBackToOwnLocalPosition)
    {
        WorldBuilder builder;
        builder.AddSystem(std::make_unique<TransformHierarchySystemEXT>());
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& parent = world->CreateEntity();
        ECS::Entity& child = world->CreateEntity();

        Transform3ComponentEXT parentTransform;
        parentTransform.TransformEXT.setPositionProperty(Vector3(50.0f, 0.0f, 0.0f));
        parent.Attach(&parentTransform);

        Transform3ComponentEXT childTransform;
        childTransform.TransformEXT.setPositionProperty(Vector3(1.0f, 0.0f, 0.0f));
        childTransform.ParentEntityIdEXT = parent.getIdProperty();
        child.Attach(&childTransform);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);
        EXPECT_EQ(childTransform.TransformEXT.getWorldPositionProperty(), Vector3(51.0f, 0.0f, 0.0f));

        childTransform.ParentEntityIdEXT = -1;
        world->Update(gameTime);

        EXPECT_EQ(childTransform.TransformEXT.getWorldPositionProperty(), Vector3(1.0f, 0.0f, 0.0f));
    }
}
