// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/CollisionResult2DTest.cs
// (4 tests). `Default_ReturnsNonIntersectingResult` is folded into the default-constructor
// checks, since C++ has no separate `default` keyword-expression distinct from
// default-construction the way C# does for a struct. `Invert` has no upstream test coverage --
// a fresh test is added for it.
#include "CNA/Extended/CollisionResult2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(CollisionResult2DTests, ConstructorWithValuesSetsFields)
    {
        const Vector2 normal(0.0f, -1.0f);
        const Vector2 minimumTranslationVector(0.0f, -5.0f);

        const CollisionResult2D result(true, normal, 5.0f, minimumTranslationVector);

        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 5.0f);
        EXPECT_EQ(result.MinimumTranslationVector, minimumTranslationVector);
    }

    TEST(CollisionResult2DTests, NoneReturnsDefaultNonIntersectingResult)
    {
        const CollisionResult2D result = CollisionResult2D::None;

        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(result.Normal, Vector2::Zero);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 0.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2::Zero);
    }

    TEST(CollisionResult2DTests, DefaultConstructedReturnsNonIntersectingResult)
    {
        const CollisionResult2D result;

        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(result.Normal, Vector2::Zero);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 0.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2::Zero);
    }

    TEST(CollisionResult2DTests, MinimumTranslationVectorEqualsNormalTimesPenetrationDepth)
    {
        const Vector2 normal(1.0f, 0.0f);
        const float penetrationDepth = 3.0f;
        const Vector2 minimumTranslationVector = normal * penetrationDepth;

        const CollisionResult2D result(true, normal, penetrationDepth, minimumTranslationVector);

        EXPECT_EQ(result.Normal * result.PenetrationDepth, result.MinimumTranslationVector);
    }

    TEST(CollisionResult2DTests, InvertNegatesNormalAndMinimumTranslationVectorButPreservesIntersectsAndPenetrationDepth)
    {
        const Vector2 normal(0.0f, -1.0f);
        const Vector2 minimumTranslationVector(0.0f, -5.0f);
        const CollisionResult2D result(true, normal, 5.0f, minimumTranslationVector);

        const CollisionResult2D inverted = result.Invert();

        EXPECT_TRUE(inverted.Intersects);
        EXPECT_EQ(inverted.Normal, -normal);
        EXPECT_FLOAT_EQ(inverted.PenetrationDepth, 5.0f);
        EXPECT_EQ(inverted.MinimumTranslationVector, -minimumTranslationVector);
    }

    TEST(CollisionResult2DTests, InvertOfNoneIsStillNone)
    {
        const CollisionResult2D inverted = CollisionResult2D::None.Invert();

        EXPECT_FALSE(inverted.Intersects);
        EXPECT_EQ(inverted.Normal, Vector2::Zero);
        EXPECT_FLOAT_EQ(inverted.PenetrationDepth, 0.0f);
        EXPECT_EQ(inverted.MinimumTranslationVector, Vector2::Zero);
    }
}
