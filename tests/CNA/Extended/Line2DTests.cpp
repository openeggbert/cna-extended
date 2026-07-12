// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's LIne2DTest.cs (13 test methods) exercises both self-contained behavior and the
// Collision2D-dependent Intersects(Ray2D/LineSegment2D/BoundingBox2D/OrientedBoundingBox2D/
// BoundingPolygon2D) overloads deferred in Line2D.hpp. These tests cover what's actually ported:
// construction, DistanceToPoint, ClosestPoint, Normalize, Intersects(Line2D)/Intersects
// (BoundingCircle2D)/Intersects(BoundingCapsule2D), Deconstruct, Equals/GetHashCode/ToString.
#include "CNA/Extended/Line2D.hpp"

#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(Line2DTests, ConstructorSetsNormalAndDistance)
    {
        const Line2D line(Vector2(0, 1), 5.0f);
        EXPECT_EQ(line.Normal, Vector2(0, 1));
        EXPECT_FLOAT_EQ(line.Distance, 5.0f);
    }

    TEST(Line2DTests, CreateFromPointAndNormalPassesThroughPoint)
    {
        const Line2D line = Line2D::CreateFromPointAndNormal(Vector2(0, 5), Vector2(0, 1));
        EXPECT_NEAR(line.DistanceToPoint(Vector2(0, 5)), 0.0f, 1e-4f);
    }

    TEST(Line2DTests, CreateFromTwoPointsPassesThroughBoth)
    {
        const Line2D line = Line2D::CreateFromTwoPoints(Vector2(0, 0), Vector2(10, 0));
        EXPECT_NEAR(line.DistanceToPoint(Vector2(0, 0)), 0.0f, 1e-4f);
        EXPECT_NEAR(line.DistanceToPoint(Vector2(10, 0)), 0.0f, 1e-4f);
    }

    TEST(Line2DTests, CreateFromTwoPointsThrowsWhenPointsCoincide)
    {
        EXPECT_THROW((void)Line2D::CreateFromTwoPoints(Vector2(1, 1), Vector2(1, 1)), std::invalid_argument);
    }

    TEST(Line2DTests, CreateFromPointAndDirectionPassesThroughPoint)
    {
        const Line2D line = Line2D::CreateFromPointAndDirection(Vector2(2, 3), Vector2(1, 0));
        EXPECT_NEAR(line.DistanceToPoint(Vector2(2, 3)), 0.0f, 1e-4f);
    }

    TEST(Line2DTests, DistanceToPointIsSignedPerpendicularDistance)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // the X axis
        EXPECT_FLOAT_EQ(line.DistanceToPoint(Vector2(0, 5)), 5.0f);
        EXPECT_FLOAT_EQ(line.DistanceToPoint(Vector2(0, -5)), -5.0f);
    }

    TEST(Line2DTests, ClosestPointProjectsOntoLine)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // the X axis
        float distanceAlongLine = 0.0f;
        const Vector2 closest = line.ClosestPoint(Vector2(3, 7), distanceAlongLine);
        EXPECT_NEAR(closest.Y, 0.0f, 1e-4f);
    }

    TEST(Line2DTests, NormalizeProducesUnitNormal)
    {
        const Line2D line(Vector2(0, 2), 4.0f);
        const Line2D normalized = Line2D::Normalize(line);
        EXPECT_NEAR(normalized.Normal.Length(), 1.0f, 1e-4f);
        EXPECT_NEAR(normalized.Distance, 2.0f, 1e-4f);
    }

    TEST(Line2DTests, InstanceNormalizeMutatesInPlace)
    {
        Line2D line(Vector2(0, 2), 4.0f);
        line.Normalize();
        EXPECT_NEAR(line.Normal.Length(), 1.0f, 1e-4f);
    }

    TEST(Line2DTests, IntersectsLineFindsIntersectionPoint)
    {
        const Line2D horizontal(Vector2(0, 1), 0.0f); // Y = 0
        const Line2D vertical(Vector2(1, 0), 0.0f);   // X = 0
        std::optional<Vector2> point;
        EXPECT_TRUE(horizontal.Intersects(vertical, point));
        ASSERT_TRUE(point.has_value());
        EXPECT_NEAR(point->X, 0.0f, 1e-4f);
        EXPECT_NEAR(point->Y, 0.0f, 1e-4f);
    }

    TEST(Line2DTests, IntersectsParallelLinesReturnsFalse)
    {
        const Line2D a(Vector2(0, 1), 0.0f);
        const Line2D b(Vector2(0, 1), 5.0f);
        std::optional<Vector2> point;
        EXPECT_FALSE(a.Intersects(b, point));
        EXPECT_FALSE(point.has_value());
    }

    TEST(Line2DTests, IntersectsCircleWithinRadius)
    {
        const Line2D line(Vector2(0, 1), 0.0f);
        const BoundingCircle2D circle(Vector2(0, 3), 5.0f);
        EXPECT_TRUE(line.Intersects(circle));
    }

    TEST(Line2DTests, IntersectsCircleOutsideRadius)
    {
        const Line2D line(Vector2(0, 1), 0.0f);
        const BoundingCircle2D circle(Vector2(0, 10), 2.0f);
        EXPECT_FALSE(line.Intersects(circle));
    }

    TEST(Line2DTests, IntersectsCapsuleCrossingSegment)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // Y = 0
        const BoundingCapsule2D capsule(Vector2(-5, -5), Vector2(5, 5), 1.0f);
        EXPECT_TRUE(line.Intersects(capsule));
    }

    TEST(Line2DTests, DeconstructReturnsNormalAndDistance)
    {
        const Line2D line(Vector2(0, 1), 5.0f);
        Vector2 normal;
        float distance = 0.0f;
        line.Deconstruct(normal, distance);
        EXPECT_EQ(normal, Vector2(0, 1));
        EXPECT_FLOAT_EQ(distance, 5.0f);
    }

    TEST(Line2DTests, EqualsAndOperators)
    {
        const Line2D a(Vector2(0, 1), 5.0f);
        const Line2D b(Vector2(0, 1), 5.0f);
        const Line2D c(Vector2(1, 0), 5.0f);

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(Line2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const Line2D a(Vector2(0, 1), 5.0f);
        const Line2D b(Vector2(0, 1), 5.0f);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(Line2DTests, ToStringContainsNormalAndDistance)
    {
        const Line2D line(Vector2(0, 1), 5.0f);
        const std::string text = line.ToString();
        EXPECT_NE(text.find("Normal"), std::string::npos);
        EXPECT_NE(text.find("Distance"), std::string::npos);
    }
}
