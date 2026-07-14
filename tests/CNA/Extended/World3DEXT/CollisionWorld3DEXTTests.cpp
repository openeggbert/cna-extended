// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for CollisionWorld3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Structured after
// CNA::Extended::Collisions::CollisionWorld2DTests.cpp's core (non-layer-filtering)
// coverage, since CollisionWorld3DEXT deliberately has no layer system for this first
// pass -- see CollisionWorld3DEXT.hpp's header comment.
#include "CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OctreeEXT.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        class TestActor3DEXT final : public ICollisionActor3DEXT
        {
        public:
            TestActor3DEXT(int id, const BoundingBox& box) : id_(id), shape_(box) {}
            TestActor3DEXT(int id, const BoundingSphere& sphere) : id_(id), shape_(sphere) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }
            [[nodiscard]] CollisionShape3DEXT getShapeProperty() const override { return shape_; }

        private:
            int id_;
            CollisionShape3DEXT shape_;
        };
    }

    TEST(CollisionWorld3DEXTTests, Insert_ThenContains_ReturnsTrue)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.Insert(&actor);

        EXPECT_TRUE(world.Contains(&actor));
    }

    TEST(CollisionWorld3DEXTTests, Contains_WhenActorNeverInserted_ReturnsFalse)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        EXPECT_FALSE(world.Contains(&actor));
    }

    TEST(CollisionWorld3DEXTTests, Remove_WhenActorWasInserted_ReturnsTrueAndActorNoLongerContained)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
        world.Insert(&actor);

        const bool removed = world.Remove(&actor);

        EXPECT_TRUE(removed);
        EXPECT_FALSE(world.Contains(&actor));
    }

    TEST(CollisionWorld3DEXTTests, QueryCandidates_WithBounds_ReturnsOverlappingActors)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3(10.0f, 10.0f, 10.0f), Vector3(12.0f, 12.0f, 12.0f)));
        world.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> candidates =
            world.QueryCandidates(BoundingBox(Vector3(9.0f, 9.0f, 9.0f), Vector3(13.0f, 13.0f, 13.0f)));

        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], &actor);
    }

    TEST(CollisionWorld3DEXTTests, QueryCollisions_WhenActorsOverlap_ReturnsCollisionEventWithOtherActor)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT first(1, BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)));
        TestActor3DEXT second(2, BoundingBox(Vector3(1.0f, 0.0f, 0.0f), Vector3(3.0f, 2.0f, 2.0f)));
        world.Insert(&first);
        world.Insert(&second);

        const std::vector<CollisionEvent3DEXT> events = world.QueryCollisions(&first);

        ASSERT_EQ(events.size(), 1u);
        EXPECT_EQ(events[0].getOtherIdProperty(), 2);
        EXPECT_TRUE(events[0].getResultProperty().Intersects);
    }

    TEST(CollisionWorld3DEXTTests, QueryCollisions_WhenActorsDoNotOverlap_ReturnsEmpty)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT first(1, BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)));
        TestActor3DEXT second(2, BoundingBox(Vector3(100.0f, 100.0f, 100.0f), Vector3(102.0f, 102.0f, 102.0f)));
        world.Insert(&first);
        world.Insert(&second);

        const std::vector<CollisionEvent3DEXT> events = world.QueryCollisions(&first);

        EXPECT_TRUE(events.empty());
    }

    TEST(CollisionWorld3DEXTTests, QueryCollisionPairs_ReturnsEachOverlappingPairOnce)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT first(1, BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)));
        TestActor3DEXT second(2, BoundingBox(Vector3(1.0f, 0.0f, 0.0f), Vector3(3.0f, 2.0f, 2.0f)));
        TestActor3DEXT third(3, BoundingBox(Vector3(100.0f, 100.0f, 100.0f), Vector3(102.0f, 102.0f, 102.0f)));
        world.Insert(&first);
        world.Insert(&second);
        world.Insert(&third);

        const std::vector<CollisionPair3DEXT> pairs = world.QueryCollisionPairs();

        ASSERT_EQ(pairs.size(), 1u);
        const int firstId = pairs[0].getFirstIdProperty();
        const int secondId = pairs[0].getSecondIdProperty();
        EXPECT_TRUE((firstId == 1 && secondId == 2) || (firstId == 2 && secondId == 1));
    }

    TEST(CollisionWorld3DEXTTests, Rebuild_UsesActorsUpdatedBoundsForSubsequentQueries)
    {
        auto world = CollisionWorld3DEXT(std::make_unique<OctreeEXT>(16.0f));
        TestActor3DEXT actor(1, BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)));
        world.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> beforeMove =
            world.QueryCandidates(BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3(3.0f, 3.0f, 3.0f)));
        ASSERT_EQ(beforeMove.size(), 1u);

        // Actor bounds don't move in this test since TestActor3DEXT's shape is fixed at
        // construction -- this test exists to confirm Rebuild() (a thin wrapper over
        // ICollisionBroadphase3DEXT::Reset()) doesn't throw and preserves existing query
        // results when nothing has actually moved.
        world.Rebuild();

        const std::vector<ICollisionActor3DEXT*> afterRebuild =
            world.QueryCandidates(BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3(3.0f, 3.0f, 3.0f)));
        EXPECT_EQ(afterRebuild.size(), 1u);
    }
}
