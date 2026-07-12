// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingCapsule2DTest.cs,
// covering the subset of upstream test cases that don't require Collision2D or LineSegment2D,
// both deferred -- see BoundingCapsule2D.hpp.
#include "CNA/Extended/BoundingCapsule2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(BoundingCapsule2DTests, Constructor)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        EXPECT_EQ(capsule.PointA, Vector2(0, 0));
        EXPECT_EQ(capsule.PointB, Vector2(4, 0));
        EXPECT_FLOAT_EQ(capsule.Radius, 1.0f);
    }

    TEST(BoundingCapsule2DTests, CenterReturnsMidpoint)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        EXPECT_EQ(capsule.getCenterProperty(), Vector2(2, 0));
    }

    TEST(BoundingCapsule2DTests, LengthAndLengthSquared)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(3, 4), 1.0f);
        EXPECT_FLOAT_EQ(capsule.getLengthProperty(), 5.0f);
        EXPECT_FLOAT_EQ(capsule.getLengthSquaredProperty(), 25.0f);
    }

    TEST(BoundingCapsule2DTests, DirectionIsUnitVector)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(3, 4), 1.0f);
        const Vector2 dir = capsule.getDirectionProperty();
        EXPECT_NEAR(dir.Length(), 1.0f, 1e-5f);
        EXPECT_NEAR(dir.X, 0.6f, 1e-5f);
        EXPECT_NEAR(dir.Y, 0.8f, 1e-5f);
    }

    TEST(BoundingCapsule2DTests, DirectionOfDegenerateCapsuleIsZero)
    {
        const BoundingCapsule2D capsule(Vector2(1, 1), Vector2(1, 1), 1.0f);
        EXPECT_EQ(capsule.getDirectionProperty(), Vector2::Zero);
    }

    TEST(BoundingCapsule2DTests, AreaCombinesRectangleAndCircle)
    {
        // length=4, radius=1: rectangle area (4 * 2*1) + circle area (pi * 1^2)
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        EXPECT_NEAR(capsule.getAreaProperty(), 8.0f + 3.14159265f, 1e-3f);
    }

    TEST(BoundingCapsule2DTests, CreateFromCenterAndDirectionNormalizesDirection)
    {
        const BoundingCapsule2D capsule = BoundingCapsule2D::CreateFromCenterAndDirection(Vector2::Zero, Vector2(2, 0), 4.0f, 1.0f);

        EXPECT_EQ(capsule.PointA, Vector2(-2, 0));
        EXPECT_EQ(capsule.PointB, Vector2(2, 0));
        EXPECT_FLOAT_EQ(capsule.Radius, 1.0f);
    }

    TEST(BoundingCapsule2DTests, TranslateMovesBothEndpoints)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const BoundingCapsule2D translated = capsule.Translate(Vector2(1, 1));

        EXPECT_EQ(translated.PointA, Vector2(1, 1));
        EXPECT_EQ(translated.PointB, Vector2(5, 1));
        EXPECT_FLOAT_EQ(translated.Radius, 1.0f);
    }

    TEST(BoundingCapsule2DTests, DeconstructReturnsComponents)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        Vector2 pointA, pointB;
        float radius;
        capsule.Deconstruct(pointA, pointB, radius);

        EXPECT_EQ(pointA, Vector2(0, 0));
        EXPECT_EQ(pointB, Vector2(4, 0));
        EXPECT_FLOAT_EQ(radius, 1.0f);
    }

    TEST(BoundingCapsule2DTests, EqualsAndOperators)
    {
        const BoundingCapsule2D a(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const BoundingCapsule2D b(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const BoundingCapsule2D c(Vector2(0, 0), Vector2(4, 0), 2.0f);

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(BoundingCapsule2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const BoundingCapsule2D a(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const BoundingCapsule2D b(Vector2(0, 0), Vector2(4, 0), 1.0f);

        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(BoundingCapsule2DTests, ToStringContainsComponents)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const std::string text = capsule.ToString();

        EXPECT_NE(text.find("PointA"), std::string::npos);
        EXPECT_NE(text.find("PointB"), std::string::npos);
        EXPECT_NE(text.find("Radius"), std::string::npos);
    }
}
