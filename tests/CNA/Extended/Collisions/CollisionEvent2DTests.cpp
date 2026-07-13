// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collisions/CollisionEvent2DTests.cs.
// Upstream's object-initializer construction (`new CollisionEvent2D { Other = other, Result = result }`)
// -> this project's constructor-parameter translation of `required ... { get; init; }` properties
// (see CollisionEvent2D.hpp). Upstream's `Assert.Same(other, collision.Other)` (reference identity)
// -> comparing the returned reference's address to the original object's address.
#include "CNA/Extended/Collisions/CollisionEvent2D.hpp"

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

    TEST(CollisionEvent2DTests, ConstructorStyleInitializationStoresOtherActorAndResult)
    {
        const TestCollisionActor other(42);
        const CollisionResult2D result(true, Vector2::UnitX, 2.0f, Vector2(2.0f, 0.0f));

        const CollisionEvent2D collision(other, result);

        EXPECT_EQ(&collision.getOtherProperty(), &other);
        EXPECT_EQ(collision.getOtherIdProperty(), 42);
        EXPECT_EQ(collision.getResultProperty().Intersects, result.Intersects);
        EXPECT_EQ(collision.getResultProperty().Normal, result.Normal);
        EXPECT_EQ(collision.getResultProperty().PenetrationDepth, result.PenetrationDepth);
        EXPECT_EQ(collision.getResultProperty().MinimumTranslationVector, result.MinimumTranslationVector);
    }
}
