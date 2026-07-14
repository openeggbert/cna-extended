// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for OctreeEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here. Structured after
// CNA::Extended::Collisions::SpatialHashTests.cpp's own coverage (Query/Insert/Remove/
// Reset/multi-cell/negative-coordinate scenarios), since OctreeEXT is SpatialHash's own
// algorithm extended to 3D -- see OctreeEXT.hpp's header comment.
#include "CNA/Extended/World3DEXT/OctreeEXT.hpp"

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

        const BoundingBox kBox(Vector3(10.0f, 10.0f, 10.0f), Vector3(20.0f, 20.0f, 20.0f));
        const BoundingBox kQueryBounds(Vector3(10.0f, 10.0f, 10.0f), Vector3(30.0f, 30.0f, 30.0f));
        const BoundingBox kMovedQueryBounds(Vector3(130.0f, 130.0f, 130.0f), Vector3(150.0f, 150.0f, 150.0f));
    }

    TEST(OctreeEXTTests, QueryWhenOneActorOverlapsQueryBoundsReturnsOneActor)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);
        EXPECT_EQ(results.size(), 1u);
    }

    TEST(OctreeEXTTests, QueryWhenTwoActorsOverlapQueryBoundsReturnsTwoActors)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT first(kBox);
        TestActor3DEXT second(kBox);

        octree.Insert(&first);
        octree.Insert(&second);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);
        EXPECT_EQ(results.size(), 2u);
    }

    TEST(OctreeEXTTests, QueryWhenActorOverlapsMultipleCellsReturnsUniqueActor)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT actor(BoundingBox(Vector3(32.0f, 32.0f, 32.0f), Vector3(96.0f, 96.0f, 96.0f)));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results =
            octree.Query(BoundingBox(Vector3(32.0f, 32.0f, 32.0f), Vector3(128.0f, 128.0f, 128.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }

    TEST(OctreeEXTTests, QueryWhenBoundsSpanCellBoundaryReturnsActorFromAllCoveredCells)
    {
        OctreeEXT octree(10.0f);
        TestActor3DEXT actor(BoundingBox(Vector3(19.0f, 0.0f, 0.0f), Vector3(20.0f, 1.0f, 1.0f)));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(BoundingBox(Vector3(9.0f, 0.0f, 0.0f), Vector3(21.0f, 1.0f, 1.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }

    TEST(OctreeEXTTests, QueryWhenBoundsUseNegativeCoordinatesReturnsActorFromNegativeCell)
    {
        OctreeEXT octree(10.0f);
        TestActor3DEXT actor(BoundingBox(Vector3(-9.0f, -9.0f, -9.0f), Vector3(-5.0f, -5.0f, -5.0f)));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results =
            octree.Query(BoundingBox(Vector3(-10.0f, -10.0f, -10.0f), Vector3(-1.0f, -1.0f, -1.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }

    TEST(OctreeEXTTests, InsertWhenActorAlreadyExistsDoesNotDuplicateStoredActor)
    {
        OctreeEXT octree(64.0f);
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

    TEST(OctreeEXTTests, RemoveWhenActorWasInsertedAndThenRemovedReturnsNoActorsFromQuery)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);
        const bool removed = octree.Remove(&actor);

        const std::vector<ICollisionActor3DEXT*> results = octree.Query(kQueryBounds);

        EXPECT_TRUE(removed);
        EXPECT_TRUE(results.empty());
    }

    TEST(OctreeEXTTests, ResetWhenActorMovesBeforeResetUsesUpdatedBoundsForQuery)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT actor(kBox);

        octree.Insert(&actor);
        actor.SetBounds(BoundingBox(Vector3(130.0f, 130.0f, 130.0f), Vector3(150.0f, 150.0f, 150.0f)));

        octree.Reset();

        const std::vector<ICollisionActor3DEXT*> oldAreaResults = octree.Query(kQueryBounds);
        const std::vector<ICollisionActor3DEXT*> movedAreaResults = octree.Query(kMovedQueryBounds);

        EXPECT_TRUE(oldAreaResults.empty());
        EXPECT_EQ(movedAreaResults.size(), 1u);
    }

    TEST(OctreeEXTTests, QueryWhenActorUsesSphereBoundsUsesBroadphaseBoundingBox)
    {
        OctreeEXT octree(64.0f);
        TestActor3DEXT actor(BoundingSphere(Vector3(100.0f, 100.0f, 100.0f), 20.0f));

        octree.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> results =
            octree.Query(BoundingBox(Vector3(80.0f, 80.0f, 80.0f), Vector3(120.0f, 120.0f, 120.0f)));

        ASSERT_EQ(results.size(), 1u);
        EXPECT_EQ(results[0], &actor);
    }
}
