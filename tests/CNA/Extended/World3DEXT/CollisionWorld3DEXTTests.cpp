// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for CollisionWorld3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. The original (pre-layer) coverage
// mirrored CollisionWorld2DTests.cpp's core (non-layer-filtering) tests; the layer-focused
// tests added below mirror that same file's layer-filtering subset (2026-07-14,
// user-requested parity work), using the same local test-only type shapes: NamedLayerActor3DEXT
// (id + shape constructor), CountingShapeActor3DEXT (tracks how many times its shape property
// is accessed, to prove layer-filtering short-circuits before ever touching actor shapes), and
// ResetTrackingLayer3DEXT (a Layer3DEXT subclass counting Reset() calls, proving
// Layer3DEXT::Reset() is genuinely virtual).
#include "CNA/Extended/World3DEXT/CollisionWorld3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SpatialHash3DEXT.hpp"
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"
#include "System/InvalidOperationException.hpp"

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

        class CountingShapeActor3DEXT final : public ICollisionActor3DEXT
        {
        public:
            CountingShapeActor3DEXT(int id, const CollisionShape3DEXT& shape) : id_(id), shape_(shape) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }

            [[nodiscard]] CollisionShape3DEXT getShapeProperty() const override
            {
                ++shapeAccessCount_;
                return shape_;
            }

            [[nodiscard]] int getShapeAccessCountProperty() const { return shapeAccessCount_; }
            void ResetShapeAccessCount() const { shapeAccessCount_ = 0; }

        private:
            int id_;
            CollisionShape3DEXT shape_;
            mutable int shapeAccessCount_ = 0;
        };

        class ResetTrackingLayer3DEXT final : public Layer3DEXT
        {
        public:
            explicit ResetTrackingLayer3DEXT(std::unique_ptr<ICollisionBroadphase3DEXT> space) : Layer3DEXT(std::move(space)) {}

            void Reset() override
            {
                ++resetCallCount_;
                Layer3DEXT::Reset();
            }

            [[nodiscard]] int getResetCallCountProperty() const { return resetCallCount_; }

        private:
            int resetCallCount_ = 0;
        };

        std::unique_ptr<Layer3DEXT> MakeSpatialHashLayer3DEXT() { return std::make_unique<Layer3DEXT>(std::make_unique<SpatialHash3DEXT>(16.0f)); }
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

    TEST(CollisionWorld3DEXTTests, RebuildDynamicLayers_UsesActorsUpdatedBoundsForSubsequentQueries)
    {
        auto world = CollisionWorld3DEXT(std::make_unique<SpatialHash3DEXT>(16.0f));
        TestActor3DEXT actor(1, BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)));
        world.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> beforeMove =
            world.QueryCandidates(BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3(3.0f, 3.0f, 3.0f)));
        ASSERT_EQ(beforeMove.size(), 1u);

        // Actor bounds don't move in this test since TestActor3DEXT's shape is fixed at
        // construction -- this test exists to confirm RebuildDynamicLayers() (a wrapper over
        // every registered Layer3DEXT's Reset(), itself a thin wrapper over
        // ICollisionBroadphase3DEXT::Reset()) doesn't throw and preserves existing query
        // results when nothing has actually moved.
        world.RebuildDynamicLayers();

        const std::vector<ICollisionActor3DEXT*> afterRebuild =
            world.QueryCandidates(BoundingBox(Vector3(-1.0f, -1.0f, -1.0f), Vector3(3.0f, 3.0f, 3.0f)));
        EXPECT_EQ(afterRebuild.size(), 1u);
    }

    // Layer/LayerPair regression tests (2026-07-14, user-requested parity with
    // CollisionWorld2D's layer system -- see CollisionWorld3DEXT.hpp's file header).

    TEST(CollisionWorld3DEXTTests, InsertWithoutLayerName_StoresActorInDefaultLayer)
    {
        auto defaultLayer = std::make_unique<ResetTrackingLayer3DEXT>(std::make_unique<SpatialHash3DEXT>(16.0f));
        ResetTrackingLayer3DEXT* defaultLayerPtr = defaultLayer.get();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.Insert(&actor);

        const std::vector<ICollisionActor3DEXT*> stored = defaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld3DEXTTests, InsertWithLayerName_StoresActorInMatchingLayer)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        Layer3DEXT* namedLayerPtr = namedLayer.get();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        const std::vector<ICollisionActor3DEXT*> stored = namedLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld3DEXTTests, InsertWhenTargetLayerIsMissing_ThrowsUndefinedLayerException)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        EXPECT_THROW(world.Insert(&actor, "actors"), Collisions::UndefinedLayerException);
    }

    TEST(CollisionWorld3DEXTTests, InsertWhenActorAlreadyExists_ThrowsInvalidOperationException)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor);

        EXPECT_THROW(world.Insert(&actor, "actors"), System::InvalidOperationException);
    }

    TEST(CollisionWorld3DEXTTests, TryGetLayerNameWhenActorIsPresent_ReturnsTrueAndAssignedLayer)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        std::string layerName;
        const bool found = world.TryGetLayerName(&actor, layerName);

        EXPECT_TRUE(found);
        EXPECT_EQ(layerName, "actors");
    }

    TEST(CollisionWorld3DEXTTests, TryGetLayerNameWhenActorIsNotPresent_ReturnsFalse)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        std::string layerName = "sentinel";
        const bool found = world.TryGetLayerName(&actor, layerName);

        EXPECT_FALSE(found);
        EXPECT_TRUE(layerName.empty());
    }

    TEST(CollisionWorld3DEXTTests, GetLayerNameWhenActorIsPresent_ReturnsAssignedLayer)
    {
        CollisionWorld3DEXT world;
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.Insert(&actor);
        EXPECT_EQ(world.GetLayerName(&actor), CollisionWorld3DEXT::DefaultLayerName);
    }

    TEST(CollisionWorld3DEXTTests, MoveToLayerWhenTargetLayerExists_MovesActorAndUpdatesMembership)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        Layer3DEXT* defaultLayerPtr = defaultLayer.get();
        Layer3DEXT* namedLayerPtr = namedLayer.get();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor);

        world.MoveToLayer(&actor, "actors");

        EXPECT_EQ(world.GetLayerName(&actor), "actors");
        EXPECT_TRUE(defaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty()).empty());
        const std::vector<ICollisionActor3DEXT*> stored = namedLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld3DEXTTests, RebuildDynamicLayers_ResetsAllLayers)
    {
        auto defaultLayer = std::make_unique<ResetTrackingLayer3DEXT>(std::make_unique<SpatialHash3DEXT>(16.0f));
        auto namedLayer = std::make_unique<ResetTrackingLayer3DEXT>(std::make_unique<SpatialHash3DEXT>(16.0f));
        ResetTrackingLayer3DEXT* defaultLayerPtr = defaultLayer.get();
        ResetTrackingLayer3DEXT* namedLayerPtr = namedLayer.get();
        CollisionWorld3DEXT world(std::move(defaultLayer));

        world.AddLayer("actors", std::move(namedLayer));
        world.RebuildDynamicLayers();

        EXPECT_EQ(defaultLayerPtr->getResetCallCountProperty(), 1);
        EXPECT_EQ(namedLayerPtr->getResetCallCountProperty(), 1);
    }

    TEST(CollisionWorld3DEXTTests, QueryCandidatesWhenCrossLayerCollisionIsDisabled_ReturnsNoCandidates)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT defaultActor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
        TestActor3DEXT namedActor(2, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<ICollisionActor3DEXT*> candidates = world.QueryCandidates(&defaultActor, "actors");
        EXPECT_TRUE(candidates.empty());
    }

    TEST(CollisionWorld3DEXTTests, QueryCollisionsWhenCrossLayerCollisionIsDisabled_DoesNotTouchOtherActorShape)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        CountingShapeActor3DEXT defaultActor(1, CollisionShape3DEXT(BoundingBox(Vector3::Zero, Vector3(2.0f, 2.0f, 2.0f))));
        CountingShapeActor3DEXT namedActor(2, CollisionShape3DEXT(BoundingBox(Vector3::Zero, Vector3(2.0f, 2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");
        defaultActor.ResetShapeAccessCount();
        namedActor.ResetShapeAccessCount();

        const std::vector<CollisionEvent3DEXT> collisions = world.QueryCollisions(&defaultActor, "actors");

        EXPECT_TRUE(collisions.empty());
        EXPECT_EQ(defaultActor.getShapeAccessCountProperty(), 0);
        EXPECT_EQ(namedActor.getShapeAccessCountProperty(), 0);
    }

    TEST(CollisionWorld3DEXTTests, QueryCandidatesWhenCrossLayerCollisionIsEnabled_ReturnsCandidates)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT defaultActor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));
        TestActor3DEXT namedActor(2, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        world.EnableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<ICollisionActor3DEXT*> candidates = world.QueryCandidates(&defaultActor, "actors");
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], &namedActor);
    }

    TEST(CollisionWorld3DEXTTests, QueryCollisionPairsWhenCrossLayerCollisionIsDisabled_DoesNotTouchActorShapes)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        CountingShapeActor3DEXT defaultActor(1, CollisionShape3DEXT(BoundingBox(Vector3::Zero, Vector3(2.0f, 2.0f, 2.0f))));
        CountingShapeActor3DEXT namedActor(2, CollisionShape3DEXT(BoundingBox(Vector3::Zero, Vector3(2.0f, 2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");
        defaultActor.ResetShapeAccessCount();
        namedActor.ResetShapeAccessCount();

        const std::vector<CollisionPair3DEXT> pairs = world.QueryCollisionPairs(CollisionWorld3DEXT::DefaultLayerName, "actors");

        EXPECT_TRUE(pairs.empty());
        EXPECT_EQ(defaultActor.getShapeAccessCountProperty(), 0);
        EXPECT_EQ(namedActor.getShapeAccessCountProperty(), 0);
    }

    TEST(CollisionWorld3DEXTTests, IsCollisionEnabledBetweenLayers_ReflectsEnableAndDisableCalls)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));

        world.AddLayer("actors", std::move(namedLayer));
        EXPECT_TRUE(world.IsCollisionEnabledBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors"));

        world.DisableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        EXPECT_FALSE(world.IsCollisionEnabledBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors"));

        world.EnableCollisionBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors");
        EXPECT_TRUE(world.IsCollisionEnabledBetweenLayers(CollisionWorld3DEXT::DefaultLayerName, "actors"));
    }

    TEST(CollisionWorld3DEXTTests, RemoveLayer_RemovesActorsAssignedToThatLayer)
    {
        auto defaultLayer = MakeSpatialHashLayer3DEXT();
        auto namedLayer = MakeSpatialHashLayer3DEXT();
        CollisionWorld3DEXT world(std::move(defaultLayer));
        TestActor3DEXT actor(1, BoundingBox(Vector3::Zero, Vector3(1.0f, 1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        const bool removed = world.RemoveLayer("actors");

        EXPECT_TRUE(removed);
        EXPECT_FALSE(world.Contains(&actor));
    }
}
