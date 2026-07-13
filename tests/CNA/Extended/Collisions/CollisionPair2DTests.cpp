// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream CollisionPair2DTest.cs exists (it is exercised only indirectly, through
// CollisionWorld2D.QueryCollisionPairs in upstream's CollisionWorld2DTests.cs). Fresh tests below.
#include "CNA/Extended/Collisions/CollisionPair2D.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collisions
{
    namespace
    {
        class TestCollisionActor : public ICollisionActor
        {
        public:
            explicit TestCollisionActor(int id) : id_(id) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }
            [[nodiscard]] CollisionShape2D getShapeProperty() const override { return CollisionShape2D(); }

        private:
            int id_;
        };
    }

    TEST(CollisionPair2DTests, StoresFirstAndSecondActorsAndTheirIds)
    {
        const TestCollisionActor first(11);
        const TestCollisionActor second(22);
        const CollisionResult2D firstResult(true, Vector2::UnitX, 3.0f, Vector2(3.0f, 0.0f));

        const CollisionPair2D pair(first, second, firstResult);

        EXPECT_EQ(&pair.getFirstProperty(), &first);
        EXPECT_EQ(&pair.getSecondProperty(), &second);
        EXPECT_EQ(pair.getFirstIdProperty(), 11);
        EXPECT_EQ(pair.getSecondIdProperty(), 22);
    }

    TEST(CollisionPair2DTests, SecondResultIsFirstResultInverted)
    {
        const TestCollisionActor first(1);
        const TestCollisionActor second(2);
        const CollisionResult2D firstResult(true, Vector2::UnitX, 3.0f, Vector2(3.0f, 0.0f));

        const CollisionPair2D pair(first, second, firstResult);

        const CollisionResult2D secondResult = pair.getSecondResultProperty();
        EXPECT_EQ(secondResult.Intersects, firstResult.Intersects);
        EXPECT_EQ(secondResult.Normal, -firstResult.Normal);
        EXPECT_EQ(secondResult.PenetrationDepth, firstResult.PenetrationDepth);
        EXPECT_EQ(secondResult.MinimumTranslationVector, -firstResult.MinimumTranslationVector);
    }
}
