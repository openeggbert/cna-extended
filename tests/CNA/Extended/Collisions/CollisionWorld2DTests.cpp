// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/CollisionWorld2DTests.cs.
// Uses the shared BasicActor fixture (Implementation/BasicActor.hpp) plus 3 local test-only types
// matching upstream's own local nested classes: NamedLayerActor (id + shape constructor),
// CountingShapeActor (tracks how many times its Shape property is accessed, to prove
// layer-filtering short-circuits before ever touching actor shapes), and ResetTrackingLayer (a
// Layer subclass counting Reset() calls, proving Layer::Reset() is genuinely virtual).
//
// C#'s `Assert.Same`/`ReferenceEquals` checks -> comparing addresses via `&`. `out string
// layerName` on failure being `Assert.Null` -> this port's TryGetLayerName clears layerName to ""
// on failure (see CollisionWorld2D.hpp's header comment on the null/empty-string sentinel
// decision) -> checked via `.empty()` instead.
#include "CNA/Extended/Collisions/CollisionWorld2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/CollisionShape2D.hpp"
#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"
#include "CNA/Extended/Collisions/Implementation/BasicActor.hpp"
#include "CNA/Extended/Collisions/Layer.hpp"
#include "CNA/Extended/Collisions/SpatialHash.hpp"
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/SizeF.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <vector>

namespace CNA::Extended::Collisions
{
    namespace
    {
        class NamedLayerActor : public ICollisionActor
        {
        public:
            NamedLayerActor(int id, const CollisionShape2D& shape) : id_(id), shape_(shape) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }
            [[nodiscard]] CollisionShape2D getShapeProperty() const override { return shape_; }

        private:
            int id_;
            CollisionShape2D shape_;
        };

        class CountingShapeActor : public ICollisionActor
        {
        public:
            CountingShapeActor(int id, const CollisionShape2D& shape) : id_(id), shape_(shape) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }

            [[nodiscard]] CollisionShape2D getShapeProperty() const override
            {
                ++shapeAccessCount_;
                return shape_;
            }

            [[nodiscard]] int getShapeAccessCountProperty() const { return shapeAccessCount_; }
            void ResetShapeAccessCount() const { shapeAccessCount_ = 0; }

        private:
            int id_;
            CollisionShape2D shape_;
            mutable int shapeAccessCount_ = 0;
        };

        class ResetTrackingLayer : public Layer
        {
        public:
            explicit ResetTrackingLayer(std::unique_ptr<ICollisionBroadphase2D> space) : Layer(std::move(space)) {}

            void Reset() override
            {
                ++resetCallCount_;
                Layer::Reset();
            }

            [[nodiscard]] int getResetCallCountProperty() const { return resetCallCount_; }

        private:
            int resetCallCount_ = 0;
        };

        std::unique_ptr<Layer> MakeSpatialHashLayer() { return std::make_unique<Layer>(std::make_unique<SpatialHash>(SizeF(64.0f, 64.0f))); }
    }

    TEST(CollisionWorld2DTests, InsertWithoutLayerNameStoresActorInDefaultLayer)
    {
        auto defaultLayer = std::make_unique<ResetTrackingLayer>(std::make_unique<SpatialHash>(SizeF(64.0f, 64.0f)));
        ResetTrackingLayer* defaultLayerPtr = defaultLayer.get();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.Insert(&actor);

        const std::vector<ICollisionActor*> stored = defaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld2DTests, InsertWithLayerNameStoresActorInMatchingLayer)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        Layer* namedLayerPtr = namedLayer.get();
        CollisionWorld2D world(std::move(defaultLayer));
        NamedLayerActor actor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        const std::vector<ICollisionActor*> stored = namedLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld2DTests, InsertWhenTargetLayerIsMissingThrowsUndefinedLayerException)
    {
        CollisionWorld2D world;
        NamedLayerActor actor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f))));

        EXPECT_THROW(world.Insert(&actor, "actors"), UndefinedLayerException);
    }

    TEST(CollisionWorld2DTests, InsertWhenActorAlreadyExistsOnDifferentLayerThrowsInvalidOperationException)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor);

        EXPECT_THROW(world.Insert(&actor, "actors"), System::InvalidOperationException);
    }

    TEST(CollisionWorld2DTests, InsertSameActorIntoDifferentWorldsAllowsBothInsertions)
    {
        auto firstDefaultLayer = MakeSpatialHashLayer();
        auto secondDefaultLayer = MakeSpatialHashLayer();
        Layer* firstDefaultLayerPtr = firstDefaultLayer.get();
        Layer* secondDefaultLayerPtr = secondDefaultLayer.get();
        CollisionWorld2D firstWorld(std::move(firstDefaultLayer));
        CollisionWorld2D secondWorld(std::move(secondDefaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        firstWorld.Insert(&actor);
        secondWorld.Insert(&actor);

        EXPECT_EQ(firstDefaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty()).size(), 1u);
        EXPECT_EQ(secondDefaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty()).size(), 1u);
    }

    TEST(CollisionWorld2DTests, ContainsWhenActorIsPresentReturnsTrue)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.Insert(&actor);
        EXPECT_TRUE(world.Contains(&actor));
    }

    TEST(CollisionWorld2DTests, ContainsWhenActorIsNotPresentReturnsFalse)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        EXPECT_FALSE(world.Contains(&actor));
    }

    TEST(CollisionWorld2DTests, TryGetLayerNameWhenActorIsPresentReturnsTrueAndAssignedLayer)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        std::string layerName;
        const bool found = world.TryGetLayerName(&actor, layerName);

        EXPECT_TRUE(found);
        EXPECT_EQ(layerName, "actors");
    }

    TEST(CollisionWorld2DTests, TryGetLayerNameWhenActorIsNotPresentReturnsFalse)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        std::string layerName = "sentinel";
        const bool found = world.TryGetLayerName(&actor, layerName);

        EXPECT_FALSE(found);
        EXPECT_TRUE(layerName.empty());
    }

    TEST(CollisionWorld2DTests, GetLayerNameWhenActorIsPresentReturnsAssignedLayer)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.Insert(&actor);
        EXPECT_EQ(world.GetLayerName(&actor), CollisionWorld2D::DefaultLayerName);
    }

    TEST(CollisionWorld2DTests, GetLayerNameWhenActorIsNotPresentThrowsInvalidOperationException)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        EXPECT_THROW((void)world.GetLayerName(&actor), System::InvalidOperationException);
    }

    TEST(CollisionWorld2DTests, MoveToLayerWhenTargetLayerExistsMovesActorAndUpdatesMembership)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        Layer* defaultLayerPtr = defaultLayer.get();
        Layer* namedLayerPtr = namedLayer.get();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor);

        world.MoveToLayer(&actor, "actors");

        EXPECT_EQ(world.GetLayerName(&actor), "actors");
        EXPECT_TRUE(defaultLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty()).empty());
        const std::vector<ICollisionActor*> stored = namedLayerPtr->getSpaceProperty().Query(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(stored.size(), 1u);
        EXPECT_EQ(stored[0], &actor);
    }

    TEST(CollisionWorld2DTests, MoveToLayerWhenActorIsNotPresentThrowsInvalidOperationException)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.AddLayer("actors", std::move(namedLayer));

        EXPECT_THROW(world.MoveToLayer(&actor, "actors"), System::InvalidOperationException);
    }

    TEST(CollisionWorld2DTests, MoveToLayerWhenTargetLayerIsMissingThrowsUndefinedLayerException)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.Insert(&actor);
        EXPECT_THROW(world.MoveToLayer(&actor, "actors"), UndefinedLayerException);
    }

    TEST(CollisionWorld2DTests, RebuildDynamicLayersResetsAllLayers)
    {
        auto defaultLayer = std::make_unique<ResetTrackingLayer>(std::make_unique<SpatialHash>(SizeF(64.0f, 64.0f)));
        auto namedLayer = std::make_unique<ResetTrackingLayer>(std::make_unique<SpatialHash>(SizeF(64.0f, 64.0f)));
        ResetTrackingLayer* defaultLayerPtr = defaultLayer.get();
        ResetTrackingLayer* namedLayerPtr = namedLayer.get();
        CollisionWorld2D world(std::move(defaultLayer));

        world.AddLayer("actors", std::move(namedLayer));
        world.RebuildDynamicLayers();

        EXPECT_EQ(defaultLayerPtr->getResetCallCountProperty(), 1);
        EXPECT_EQ(namedLayerPtr->getResetCallCountProperty(), 1);
    }

    TEST(CollisionWorld2DTests, RebuildDynamicLayersWhenLayerIsStaticStillCallsReset)
    {
        auto defaultLayer = std::make_unique<ResetTrackingLayer>(std::make_unique<SpatialHash>(SizeF(64.0f, 64.0f)));
        defaultLayer->setIsDynamicProperty(false);
        ResetTrackingLayer* defaultLayerPtr = defaultLayer.get();
        CollisionWorld2D world(std::move(defaultLayer));

        world.RebuildDynamicLayers();
        EXPECT_EQ(defaultLayerPtr->getResetCallCountProperty(), 1);
    }

    TEST(CollisionWorld2DTests, QueryCandidatesWhenLayerNameIsEmptyUsesDefaultLayerBroadphase)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor actor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));

        world.Insert(&actor);

        const std::vector<ICollisionActor*> candidates = world.QueryCandidates(actor.getShapeProperty().getBoundingBoxProperty());
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], &actor);
    }

    TEST(CollisionWorld2DTests, QueryCandidatesWhenNamedLayerIsProvidedUsesMatchingLayerBroadphase)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        NamedLayerActor actor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&actor, "actors");

        const std::vector<ICollisionActor*> candidates = world.QueryCandidates(actor.getShapeProperty().getBoundingBoxProperty(), "actors");
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], &actor);
    }

    TEST(CollisionWorld2DTests, QueryCandidatesWhenCrossLayerCollisionIsDisabledReturnsNoCandidates)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));
        NamedLayerActor namedActor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld2D::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<ICollisionActor*> candidates = world.QueryCandidates(&defaultActor, "actors");
        EXPECT_TRUE(candidates.empty());
    }

    TEST(CollisionWorld2DTests, QueryCollisionsWhenCrossLayerCollisionIsDisabledDoesNotTouchOtherActorShape)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        CountingShapeActor defaultActor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f))));
        CountingShapeActor namedActor(2, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld2D::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");
        defaultActor.ResetShapeAccessCount();
        namedActor.ResetShapeAccessCount();

        const std::vector<CollisionEvent2D> collisions = world.QueryCollisions(&defaultActor, "actors");

        EXPECT_TRUE(collisions.empty());
        EXPECT_EQ(defaultActor.getShapeAccessCountProperty(), 0);
        EXPECT_EQ(namedActor.getShapeAccessCountProperty(), 0);
    }

    TEST(CollisionWorld2DTests, QueryCandidatesWhenCrossLayerCollisionIsEnabledReturnsCandidates)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f)));
        NamedLayerActor namedActor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(1.0f, 1.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld2D::DefaultLayerName, "actors");
        world.EnableCollisionBetweenLayers(CollisionWorld2D::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<ICollisionActor*> candidates = world.QueryCandidates(&defaultActor, "actors");
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], &namedActor);
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenCrossLayerCollisionIsDisabledDoesNotTouchActorShapes)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        CountingShapeActor defaultActor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f))));
        CountingShapeActor namedActor(2, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.DisableCollisionBetweenLayers(CollisionWorld2D::DefaultLayerName, "actors");
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");
        defaultActor.ResetShapeAccessCount();
        namedActor.ResetShapeAccessCount();

        const std::vector<CollisionPair2D> pairs = world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, "actors");

        EXPECT_TRUE(pairs.empty());
        EXPECT_EQ(defaultActor.getShapeAccessCountProperty(), 0);
        EXPECT_EQ(namedActor.getShapeAccessCountProperty(), 0);
    }

    TEST(CollisionWorld2DTests, QueryCollisionsWhenShapesOverlapReturnsCollisionResult)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f)));
        NamedLayerActor namedActor(1, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2(1.0f, 0.0f), Vector2(2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<CollisionEvent2D> collisions = world.QueryCollisions(&defaultActor, "actors");
        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(&collisions[0].getOtherProperty(), &namedActor);
        EXPECT_TRUE(collisions[0].getResultProperty().Intersects);
    }

    TEST(CollisionWorld2DTests, QueryCollisionsWhenCandidateDoesNotProduceCollisionResultSkipsCandidate)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f)));
        NamedLayerActor namedActor(1, CollisionShape2D(BoundingCapsule2D(Vector2(1.0f, -1.0f), Vector2(1.0f, 1.0f), 0.5f)));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<CollisionEvent2D> collisions = world.QueryCollisions(&defaultActor, "actors");
        EXPECT_TRUE(collisions.empty());
    }

    TEST(CollisionWorld2DTests, QueryCollisionsWhenCircleAndBoxShapesOverlapReturnsCollisionResult)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        NamedLayerActor circleActor(1, CollisionShape2D(BoundingCircle2D(Vector2::Zero, 2.0f)));
        NamedLayerActor boxActor(2, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2(1.0f, -2.0f), Vector2(4.0f, 4.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&circleActor);
        world.Insert(&boxActor, "actors");

        const std::vector<CollisionEvent2D> collisions = world.QueryCollisions(&circleActor, "actors");
        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(&collisions[0].getOtherProperty(), &boxActor);
        EXPECT_TRUE(collisions[0].getResultProperty().Intersects);
        EXPECT_NE(collisions[0].getResultProperty().MinimumTranslationVector, Vector2::Zero);
    }

    TEST(CollisionWorld2DTests, QueryCollisionsWhenObbAndBoxShapesOverlapReturnsCollisionResult)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        NamedLayerActor obbActor(
            1, CollisionShape2D(OrientedBoundingBox2D::CreateFromRotation(
                   Vector2(3.0f, 2.0f), Microsoft::Xna::Framework::MathHelper::PiOver4, Vector2(2.0f, 2.0f))));
        NamedLayerActor boxActor(2, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(4.0f, 4.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&obbActor);
        world.Insert(&boxActor, "actors");

        const std::vector<CollisionEvent2D> collisions = world.QueryCollisions(&obbActor, "actors");
        ASSERT_EQ(collisions.size(), 1u);
        EXPECT_EQ(&collisions[0].getOtherProperty(), &boxActor);
        EXPECT_TRUE(collisions[0].getResultProperty().Intersects);
        EXPECT_NE(collisions[0].getResultProperty().MinimumTranslationVector, Vector2::Zero);
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenWorldContainsMixedShapesReturnsOnlySupportedPairs)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        NamedLayerActor circleActor(1, CollisionShape2D(BoundingCircle2D(Vector2::Zero, 2.0f)));
        NamedLayerActor polygonActor(
            2, CollisionShape2D(BoundingPolygon2D(
                   std::vector<Vector2>{Vector2(1.0f, -1.0f), Vector2(3.0f, -1.0f), Vector2(3.0f, 1.0f), Vector2(1.0f, 1.0f)},
                   std::vector<Vector2>{-Vector2::UnitY, Vector2::UnitX, Vector2::UnitY, -Vector2::UnitX})));
        NamedLayerActor unsupportedCapsuleActor(3, CollisionShape2D(BoundingCapsule2D(Vector2(0.5f, -1.0f), Vector2(0.5f, 1.0f), 0.5f)));

        world.Insert(&circleActor);
        world.Insert(&polygonActor);
        world.Insert(&unsupportedCapsuleActor);

        const std::vector<CollisionPair2D> pairs =
            world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, CollisionWorld2D::DefaultLayerName);

        ASSERT_EQ(pairs.size(), 1u);
        EXPECT_EQ(pairs[0].getFirstIdProperty(), circleActor.getIdProperty());
        EXPECT_EQ(pairs[0].getSecondIdProperty(), unsupportedCapsuleActor.getIdProperty());
        EXPECT_TRUE(pairs[0].getFirstResultProperty().Intersects);
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenCollisionDataIsNeededReturnsPairWithResults)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f)));
        NamedLayerActor namedActor(7, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2(1.0f, 0.0f), Vector2(2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<CollisionPair2D> pairs = world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, "actors");
        ASSERT_EQ(pairs.size(), 1u);
        EXPECT_EQ(&pairs[0].getFirstProperty(), &defaultActor);
        EXPECT_EQ(&pairs[0].getSecondProperty(), &namedActor);
        EXPECT_TRUE(pairs[0].getFirstResultProperty().Intersects);
        EXPECT_EQ(pairs[0].getFirstIdProperty(), defaultActor.getIdProperty());
        EXPECT_EQ(pairs[0].getSecondIdProperty(), namedActor.getIdProperty());
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenSecondResultIsRequestedReturnsOppositeResultDirection)
    {
        auto defaultLayer = MakeSpatialHashLayer();
        auto namedLayer = MakeSpatialHashLayer();
        CollisionWorld2D world(std::move(defaultLayer));
        Tests::BasicActor defaultActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f)));
        NamedLayerActor namedActor(7, CollisionShape2D(BoundingBox2D::CreateFromPositionAndSize(Vector2(1.0f, 0.0f), Vector2(2.0f, 2.0f))));

        world.AddLayer("actors", std::move(namedLayer));
        world.Insert(&defaultActor);
        world.Insert(&namedActor, "actors");

        const std::vector<CollisionPair2D> pairs = world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, "actors");
        ASSERT_EQ(pairs.size(), 1u);
        EXPECT_EQ(-pairs[0].getFirstResultProperty().Normal, pairs[0].getSecondResultProperty().Normal);
        EXPECT_EQ(pairs[0].getFirstResultProperty().PenetrationDepth, pairs[0].getSecondResultProperty().PenetrationDepth);
        EXPECT_EQ(-pairs[0].getFirstResultProperty().MinimumTranslationVector, pairs[0].getSecondResultProperty().MinimumTranslationVector);
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenActorsShareLayerReturnsPairOnlyOnce)
    {
        CollisionWorld2D world(MakeSpatialHashLayer());
        Tests::BasicActor firstActor(BoundingBox2D::CreateFromPositionAndSize(Vector2::Zero, Vector2(2.0f, 2.0f)));
        Tests::BasicActor secondActor(BoundingBox2D::CreateFromPositionAndSize(Vector2(1.0f, 0.0f), Vector2(2.0f, 2.0f)));

        world.Insert(&firstActor);
        world.Insert(&secondActor);

        const std::vector<CollisionPair2D> pairs =
            world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, CollisionWorld2D::DefaultLayerName);

        ASSERT_EQ(pairs.size(), 1u);
        EXPECT_EQ(pairs[0].getFirstIdProperty(), firstActor.getIdProperty());
        EXPECT_EQ(pairs[0].getSecondIdProperty(), secondActor.getIdProperty());
    }

    TEST(CollisionWorld2DTests, QueryCollisionPairsWhenActorsSpanMultipleBroadphaseCellsSuppressesDuplicates)
    {
        CollisionWorld2D world(std::make_unique<Layer>(std::make_unique<SpatialHash>(SizeF(32.0f, 32.0f))));
        Tests::BasicActor firstActor(BoundingBox2D::CreateFromPositionAndSize(Vector2(16.0f, 16.0f), Vector2(48.0f, 48.0f)));
        Tests::BasicActor secondActor(BoundingBox2D::CreateFromPositionAndSize(Vector2(32.0f, 32.0f), Vector2(32.0f, 32.0f)));

        world.Insert(&firstActor);
        world.Insert(&secondActor);

        const std::vector<CollisionPair2D> pairs =
            world.QueryCollisionPairs(CollisionWorld2D::DefaultLayerName, CollisionWorld2D::DefaultLayerName);

        ASSERT_EQ(pairs.size(), 1u);
        EXPECT_EQ(pairs[0].getFirstIdProperty(), firstActor.getIdProperty());
        EXPECT_EQ(pairs[0].getSecondIdProperty(), secondActor.getIdProperty());
    }
}
