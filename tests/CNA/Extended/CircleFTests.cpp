// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended's own tests/MonoGame.Extended.Tests/Primitives/CircleFTests.cs has only one
// active test (the rest is commented out upstream itself, not disabled by this port) -- ported
// 1:1 below (CircCircIntersectionDiagonalCircleTest). Everything else here is a fresh test
// covering what's actually ported, matching the depth of BoundingCircle2DTests.cpp. The
// Intersects(CircleF, BoundingRectangle) overloads are deferred (see CircleF.hpp) so not tested.
#include "CNA/Extended/CircleF.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    // Ported 1:1 from upstream's one active test.
    TEST(CircleFTests, CircCircIntersectionDiagonalCircleTest)
    {
        const CircleF circle(Vector2(16.0f, 16.0f), 16.0f);
        const Vector2 point(0, 0);

        EXPECT_FALSE(circle.Contains(point));
    }

    TEST(CircleFTests, ConstructorSetsCenterAndRadius)
    {
        const CircleF circle(Vector2(5, 5), 15.0f);
        EXPECT_EQ(circle.Center, Vector2(5, 5));
        EXPECT_FLOAT_EQ(circle.Radius, 15.0f);
    }

    TEST(CircleFTests, DiameterIsTwiceRadius)
    {
        const CircleF circle(Vector2::Zero, 10.0f);
        EXPECT_FLOAT_EQ(circle.getDiameterProperty(), 20.0f);
    }

    TEST(CircleFTests, CreateFromMinimumMaximum)
    {
        const CircleF circle = CircleF::CreateFrom(Vector2(5, 5), Vector2(15, 15));
        EXPECT_EQ(circle.Center, Vector2(10, 10));
        EXPECT_FLOAT_EQ(circle.Radius, 5.0f);
    }

    TEST(CircleFTests, CreateFromPoints)
    {
        const std::vector<Vector2> points = {
            Vector2(5, 5), Vector2(10, 10), Vector2(15, 15), Vector2(-5, -5), Vector2(-15, -15)};
        const CircleF circle = CircleF::CreateFrom(points);
        EXPECT_EQ(circle.Center, Vector2(0, 0));
        EXPECT_FLOAT_EQ(circle.Radius, 15.0f);
    }

    TEST(CircleFTests, CreateFromEmptyPointsIsDefaultCircle)
    {
        const CircleF circle = CircleF::CreateFrom(std::vector<Vector2>{});
        EXPECT_EQ(circle.Center, Vector2::Zero);
        EXPECT_FLOAT_EQ(circle.Radius, 0.0f);
    }

    TEST(CircleFTests, IntersectsOverlappingCircles)
    {
        const CircleF a(Vector2(-10, -10), 15.0f);
        const CircleF b(Vector2(20, 20), 40.0f);
        EXPECT_TRUE(a.Intersects(b));
        EXPECT_TRUE(CircleF::Intersects(a, b));
    }

    TEST(CircleFTests, DoesNotIntersectNonOverlappingCircles)
    {
        const CircleF a(Vector2(-40, -50), 15.0f);
        const CircleF b(Vector2(20, 20), 15.0f);
        EXPECT_FALSE(a.Intersects(b));
        EXPECT_FALSE(CircleF::Intersects(a, b));
    }

    TEST(CircleFTests, ContainsPointInside)
    {
        // Note: upstream's own (disabled/commented-out) ContainsPointTestCases used
        // Vector2(-15, -15) here, but that point is ~21.2 units from a radius-15 circle at the
        // origin -- outside it, not inside. That disabled test data was itself wrong (never ran,
        // never caught). Using an actually-inside point instead.
        const CircleF circle(Vector2(0, 0), 15.0f);
        EXPECT_TRUE(circle.Contains(Vector2(-10, -10)));
        EXPECT_TRUE(CircleF::Contains(circle, Vector2(-10, -10)));
    }

    TEST(CircleFTests, DoesNotContainPointOutside)
    {
        const CircleF circle(Vector2(0, 0), 15.0f);
        EXPECT_FALSE(circle.Contains(Vector2(-16, 15)));
    }

    TEST(CircleFTests, ClosestPointToInsidePointIsThatPoint)
    {
        const CircleF circle(Vector2(0, 0), 50.0f);
        EXPECT_EQ(circle.ClosestPointTo(Vector2(25, 25)), Vector2(25, 25));
    }

    TEST(CircleFTests, ClosestPointToOutsidePointIsOnBoundary)
    {
        const CircleF circle(Vector2(0, 0), 50.0f);
        EXPECT_EQ(circle.ClosestPointTo(Vector2(400, 0)), Vector2(50, 0));
    }

    TEST(CircleFTests, BoundaryPointAtAngle)
    {
        const CircleF circle(Vector2(0, 0), 50.0f);
        const Vector2 boundary = circle.BoundaryPointAt(MathHelper::PiOver2);
        EXPECT_NEAR(boundary.X, 0.0f, 0.001f);
        EXPECT_NEAR(boundary.Y, 50.0f, 0.001f);
    }

    TEST(CircleFTests, EqualsAndOperators)
    {
        const CircleF a(Vector2(0, 0), 50.0f);
        const CircleF b(Vector2(0, 0), 50.0f);
        const CircleF c(Vector2(50, 50), 50.0f);
        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
        EXPECT_TRUE(a.Equals(b));
    }

    TEST(CircleFTests, GetHashCodeMatchesForEqualInstances)
    {
        const CircleF a(Vector2(0, 0), 50.0f);
        const CircleF b(Vector2(0, 0), 50.0f);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(CircleFTests, ToRectangleConvertsToIntegerRectangle)
    {
        const CircleF circle(Vector2(25, 25), 25.0f);
        const Rectangle rect = circle.ToRectangle();
        EXPECT_EQ(rect, Rectangle(0, 0, 50, 50));
    }

    TEST(CircleFTests, ToRectangleFConvertsToFloatRectangle)
    {
        const CircleF circle(Vector2(25, 25), 25.0f);
        const RectangleF rect = circle.ToRectangleF();
        EXPECT_FLOAT_EQ(rect.X, 0.0f);
        EXPECT_FLOAT_EQ(rect.Y, 0.0f);
        EXPECT_FLOAT_EQ(rect.Width, 50.0f);
        EXPECT_FLOAT_EQ(rect.Height, 50.0f);
    }

    TEST(CircleFTests, FromRectangleConvertsToCircle)
    {
        const CircleF circle(Rectangle(0, 0, 50, 50));
        EXPECT_EQ(circle.Center, Vector2(25, 25));
        EXPECT_FLOAT_EQ(circle.Radius, 25.0f);
    }

    TEST(CircleFTests, FromRectangleFConvertsToCircle)
    {
        const CircleF circle(RectangleF(0.0f, 0.0f, 50.0f, 50.0f));
        EXPECT_EQ(circle.Center, Vector2(25, 25));
        EXPECT_FLOAT_EQ(circle.Radius, 25.0f);
    }
}
