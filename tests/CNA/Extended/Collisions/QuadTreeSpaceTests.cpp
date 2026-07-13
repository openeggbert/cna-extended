// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/QuadTreeSpaceTests.cs.
// `IEnumerable<ICollisionActor>`/`List<ICollisionActor>` query results -> std::vector<ICollisionActor*>
// (per ICollisionBroadphase2D.hpp's established convention). `Assert.Same(actor, collisions[0])`
// (C# reference identity) -> comparing addresses.
#include "CNA/Extended/Collisions/QuadTreeSpace.hpp"

#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Collisions/Implementation/BasicActor.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collisions
{
    namespace
    {
        using Microsoft::Xna::Framework::MathHelper;
        using Tests::BasicActor;

        const BoundingBox2D kBoundary(Vector2(0.0f, 0.0f), Vector2(256.0f, 256.0f));
        const BoundingBox2D kQueryBounds(Vector2(10.0f, 10.0f), Vector2(30.0f, 30.0f));
        const BoundingBox2D kMovedQueryBounds(Vector2(130.0f, 130.0f), Vector2(150.0f, 150.0f));

        BoundingBox2D MakeBox()
        {
            return BoundingBox2D::CreateFromPositionAndSize(Vector2(10.0f, 10.0f), Vector2(20.0f, 20.0f));
        }
    }

    TEST(QuadTreeSpaceTests, InsertWhenActorIsInsertedAndQueriedReturnsActor)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(MakeBox());

        space.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = space.Query(kQueryBounds);

        EXPECT_EQ(collisions.size(), 1u);
    }

    TEST(QuadTreeSpaceTests, RemoveWhenActorWasInsertedAndThenRemovedReturnsNoActorsFromQuery)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(MakeBox());

        space.Insert(&actor);
        const bool removed = space.Remove(&actor);

        const std::vector<ICollisionActor*> collisions = space.Query(kQueryBounds);

        EXPECT_TRUE(removed);
        EXPECT_TRUE(collisions.empty());
    }

    TEST(QuadTreeSpaceTests, ResetWhenActorMovesBeforeResetUsesUpdatedBoundsForQuery)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(MakeBox());

        space.Insert(&actor);
        actor.SetBounds(BoundingBox2D::CreateFromPositionAndSize(Vector2(130.0f, 130.0f), Vector2(20.0f, 20.0f)));

        space.Reset();

        const std::vector<ICollisionActor*> oldAreaCollisions = space.Query(kQueryBounds);
        const std::vector<ICollisionActor*> movedAreaCollisions = space.Query(kMovedQueryBounds);

        EXPECT_TRUE(oldAreaCollisions.empty());
        EXPECT_EQ(movedAreaCollisions.size(), 1u);
    }

    TEST(QuadTreeSpaceTests, QueryWhenActorOverlapsMultipleQuadrantsReturnsUniqueActor)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2(120.0f, 120.0f), Vector2(32.0f, 32.0f)));

        space.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = space.Query(BoundingBox2D(Vector2(120.0f, 120.0f), Vector2(152.0f, 152.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(QuadTreeSpaceTests, QueryWhenActorUsesCircleBoundsUsesBroadphaseBoundingBox)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(BoundingCircle2D(Vector2(100.0f, 100.0f), 20.0f));

        space.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = space.Query(BoundingBox2D(Vector2(80.0f, 80.0f), Vector2(120.0f, 120.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(QuadTreeSpaceTests, QueryWhenActorUsesOrientedRectangleBoundsCanReturnBroadphaseFalsePositive)
    {
        QuadTreeSpace space(kBoundary);
        BasicActor actor(OrientedBoundingBox2D::CreateFromRotation(Vector2(128.0f, 128.0f), MathHelper::PiOver4, Vector2(48.0f, 48.0f)));

        space.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = space.Query(BoundingBox2D(Vector2(94.0f, 94.0f), Vector2(100.0f, 100.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }
}
