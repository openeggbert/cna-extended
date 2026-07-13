// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/SpatialHashTests.cs,
// using the shared BasicActor test fixture. Upstream's `foreach (ICollisionActor storedActor in
// hash)` (desugared to hash.GetEnumerator()) -> hash.GetActors(), matching
// ICollisionBroadphase2D.hpp's established GetEnumerator()->GetActors() translation. `IEnumerable<
// ICollisionActor> collisions` -> `std::vector<ICollisionActor*> collisions` (Query's C++ return
// type is already eager, see ICollisionBroadphase2D.hpp).
#include "CNA/Extended/Collisions/SpatialHash.hpp"

#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Collisions/Implementation/BasicActor.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collisions
{
    using Microsoft::Xna::Framework::MathHelper;
    using Tests::BasicActor;

    namespace
    {
        SpatialHash CreateSpatialHash()
        {
            return SpatialHash(SizeF(64.0f, 64.0f));
        }

        const BoundingBox2D kBox = BoundingBox2D::CreateFromPositionAndSize(Vector2(10.0f, 10.0f), Vector2(20.0f, 20.0f));
        const BoundingBox2D kQueryBounds(Vector2(10.0f, 10.0f), Vector2(30.0f, 30.0f));
        const BoundingBox2D kMovedQueryBounds(Vector2(130.0f, 130.0f), Vector2(150.0f, 150.0f));
    }

    TEST(SpatialHashTests, QueryWhenOneActorOverlapsQueryBoundsReturnsOneActor)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(kBox);

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(kQueryBounds);
        EXPECT_EQ(collisions.size(), 1u);
    }

    TEST(SpatialHashTests, QueryWhenTwoActorsOverlapQueryBoundsReturnsTwoActors)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor first(kBox);
        BasicActor second(kBox);

        hash.Insert(&first);
        hash.Insert(&second);

        const std::vector<ICollisionActor*> collisions = hash.Query(kQueryBounds);
        EXPECT_EQ(collisions.size(), 2u);
    }

    TEST(SpatialHashTests, QueryWhenActorOverlapsMultipleCellsReturnsUniqueActor)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2(32.0f, 32.0f), Vector2(96.0f, 96.0f)));

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(BoundingBox2D(Vector2(32.0f, 32.0f), Vector2(128.0f, 128.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(SpatialHashTests, QueryWhenBoundsSpanCellBoundaryReturnsActorsFromAllCoveredCells)
    {
        SpatialHash hash(SizeF(10.0f, 10.0f));
        BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2(19.0f, 0.0f), Vector2(1.0f, 1.0f)));

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(BoundingBox2D(Vector2(9.0f, 0.0f), Vector2(21.0f, 1.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(SpatialHashTests, QueryWhenBoundsUseNegativeCoordinatesReturnsActorFromNegativeCell)
    {
        SpatialHash hash(SizeF(10.0f, 10.0f));
        BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2(-9.0f, -9.0f), Vector2(4.0f, 4.0f)));

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(BoundingBox2D(Vector2(-10.0f, -10.0f), Vector2(-1.0f, -1.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(SpatialHashTests, InsertWhenActorAlreadyExistsDoesNotDuplicateStoredActor)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(kBox);

        hash.Insert(&actor);
        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(kQueryBounds);
        const std::vector<ICollisionActor*>& storedActors = hash.GetActors();

        ASSERT_EQ(collisions.size(), 1u);
        ASSERT_EQ(storedActors.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
        EXPECT_EQ(storedActors[0], &actor);
    }

    TEST(SpatialHashTests, RemoveWhenActorWasInsertedAndThenRemovedReturnsNoActorsFromQuery)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(kBox);

        hash.Insert(&actor);
        const bool removed = hash.Remove(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(kQueryBounds);

        EXPECT_TRUE(removed);
        EXPECT_TRUE(collisions.empty());
    }

    TEST(SpatialHashTests, ResetWhenActorMovesBeforeResetUsesUpdatedBoundsForQuery)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(kBox);

        hash.Insert(&actor);
        actor.SetBounds(BoundingBox2D::CreateFromPositionAndSize(Vector2(130.0f, 130.0f), Vector2(20.0f, 20.0f)));

        hash.Reset();

        const std::vector<ICollisionActor*> oldAreaCollisions = hash.Query(kQueryBounds);
        const std::vector<ICollisionActor*> movedAreaCollisions = hash.Query(kMovedQueryBounds);

        EXPECT_TRUE(oldAreaCollisions.empty());
        EXPECT_EQ(movedAreaCollisions.size(), 1u);
    }

    TEST(SpatialHashTests, QueryWhenActorUsesCircleBoundsUsesBroadphaseBoundingBox)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(BoundingCircle2D(Vector2(100.0f, 100.0f), 20.0f));

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(BoundingBox2D(Vector2(80.0f, 80.0f), Vector2(120.0f, 120.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }

    TEST(SpatialHashTests, QueryWhenActorUsesOrientedRectangleBoundsCanReturnBroadphaseFalsePositive)
    {
        SpatialHash hash = CreateSpatialHash();
        BasicActor actor(OrientedBoundingBox2D::CreateFromRotation(Vector2(128.0f, 128.0f), MathHelper::PiOver4, Vector2(48.0f, 48.0f)));

        hash.Insert(&actor);

        const std::vector<ICollisionActor*> collisions = hash.Query(BoundingBox2D(Vector2(94.0f, 94.0f), Vector2(100.0f, 100.0f)));

        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(collisions[0], &actor);
    }
}
