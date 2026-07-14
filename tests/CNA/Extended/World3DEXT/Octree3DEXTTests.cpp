// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Octree3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here. Mirrors
// CNA::Extended::Collisions::QuadTreeSpaceTests.cpp's coverage (2026-07-14, user-requested
// parity work), matching SpatialHash3DEXTTests.cpp's own established 3D adaptation pattern
// (sphere bounds in place of 2D's circle/oriented-rectangle bounds cases, since
// CollisionShape3DEXT has no oriented-box shape).
#include "CNA/Extended/World3DEXT/Octree3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        class TestActor3DEXT final : public ICollisionActor3DEXT
        {
        public:
            explicit TestActor3DEXT(const BoundingBox& box, int id = 0) : id_(id), shape_(box) {}
            explicit TestActor3DEXT(const BoundingSphere& sphere, int id = 0) : id_(id), shape_(sphere) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }
            [[nodiscard]] CollisionShape3DEXT getShapeProperty() const override { return shape_; }

            void SetBounds(const BoundingBox& box) { shape_ = CollisionShape3DEXT(box); }

        private:
            int id_;
            CollisionShape3DEXT shape_;
        };

        const BoundingBox kBoundary(Vector3(0.0f, 0.0f, 0.0f), Vector3(256.0f, 256.0f, 256.0f));
        const BoundingBox kQueryBounds(Vector3(10.0f, 10.0f, 10.0f), Vector3(30.0f, 30.0f, 30.0f));
        const BoundingBox kMovedQueryBounds(Vector3(130.0f, 130.0f, 130.0f), Vector3(150.0f, 150.0f, 150.0f));
        const BoundingBox kBox(Vector3(10.0f, 10.0f, 10.0f), Vector3(20.0f, 20.0f, 20.0f));
    }

    TEST(Octree3DEXTTests, QueryWhenOneActorOverlapsQueryBoundsReturnsOneActor)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);
        EXPECT_EQ(results.size(), 1u);
    }

    TEST(Octree3DEXTTests, QueryWhenTwoActorsOverlapQueryBoundsReturnsTwoActors)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT first(kBox, 1);
        TestActor3DEXT second(kBox, 2);

        octree.Insert(&first);
        octree.Insert(&second);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);
        EXPECT_EQ(results.size(), 2u);
    }

    TEST(Octree3DEXTTests, QueryWhenActorOverlapsMultipleOctantsReturnsUniqueActor)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(BoundingBox(Vector3(120.0f, 120.0f, 120.0f), Vector3(152.0f, 152.0f, 152.0f)));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results =
            octree.Query(BoundingBox(Vector3(120.0f, 120.0f, 120.0f), Vector3(152.0f, 152.0f, 152.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }

    TEST(Octree3DEXTTests, InsertWhenActorAlreadyExistsDoesNotDuplicateStoredActor)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);
        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);
        const std::vector<ICollisionActor3DEXT*>& stored = octree.GetActors();

        ASSERT_EQ(results.size(), 1u);
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(results[0], &actor);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(Octree3DEXTTests, RemoveWhenActorWasInsertedAndThenRemovedReturnsNoActorsFromQuery)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);
        const bool removed = octree.Remove(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);

        EXPECT_TRUE(removed);
        EXPECT_TRUE(results.empty());
    }

    TEST(Octree3DEXTTests, ResetWhenActorMovesBeforeResetUsesUpdatedBoundsForQuery)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);
        actor.SetBounds(BoundingBox(Vector3(130.0f, 130.0f, 130.0f), Vector3(150.0f, 150.0f, 150.0f)));

        octree.Reset();

        const std::vector<ICollisionActor3DEXT*> oldAreaResults = octree.Query(kQueryBounds);
        const std::vector<ICollisionActor3DEXT*> movedAreaResults = octree.Query(kMovedQueryBounds);

        EXPECT_TRUE(oldAreaResults.empty());
        EXPECT_EQ(movedAreaResults.size(), 1u);
    }

    TEST(Octree3DEXTTests, QueryWhenActorUsesSphereBoundsUsesBroadphaseBoundingBox)
    {
        Octree3DEXT octree(kBoundary);
        TestActor3DEXT actor(BoundingSphere(Vector3(100.0f, 100.0f, 100.0f), 20.0f));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results =
            octree.Query(BoundingBox(Vector3(80.0f, 80.0f, 80.0f), Vector3(120.0f, 120.0f, 120.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }
}
