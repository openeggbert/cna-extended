// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's LIne2DTest.cs (13 test methods) exercises only the self-contained behavior: it never
// tests Intersects(Ray2D)/Intersects(LineSegment2D)/Intersects(BoundingBox2D)/
// Intersects(OrientedBoundingBox2D)/Intersects(BoundingPolygon2D). Those overloads are ported
// below (now that Collision2D exists) with fresh tests -- one true case and one false case per
// overload, spot-checking the delegation rather than re-deriving Collision2D's own algorithm
// correctness (already covered exhaustively by Collision2DTests.cpp).
#include "CNA/Extended/Line2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"

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

    TEST(Line2DTests, IntersectsRayInForwardDirection)
    {
        const Line2D line(Vector2(1, 0), 5.0f); // X = 5
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        std::optional<float> distanceAlongRay;
        std::optional<Vector2> point;
        EXPECT_TRUE(line.Intersects(ray, distanceAlongRay, point));
        ASSERT_TRUE(distanceAlongRay.has_value());
        EXPECT_NEAR(*distanceAlongRay, 5.0f, 1e-4f);
    }

    TEST(Line2DTests, IntersectsRayBehindOriginReturnsFalse)
    {
        const Line2D line(Vector2(1, 0), -5.0f); // X = -5
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        EXPECT_FALSE(line.Intersects(ray));
    }

    TEST(Line2DTests, IntersectsLineSegmentCrossingBounds)
    {
        const Line2D line(Vector2(1, 0), 0.0f); // X = 0
        const LineSegment2D segment(Vector2(-5, 0), Vector2(5, 0));
        std::optional<float> distanceAlongSegment;
        std::optional<Vector2> point;
        EXPECT_TRUE(line.Intersects(segment, distanceAlongSegment, point));
        ASSERT_TRUE(distanceAlongSegment.has_value());
        EXPECT_NEAR(*distanceAlongSegment, 0.5f, 1e-4f);
    }

    TEST(Line2DTests, IntersectsLineSegmentOutsideBoundsReturnsFalse)
    {
        const Line2D line(Vector2(1, 0), 10.0f); // X = 10
        const LineSegment2D segment(Vector2(-5, 0), Vector2(5, 0));
        EXPECT_FALSE(line.Intersects(segment));
    }

    TEST(Line2DTests, IntersectsBoundingBoxPassingThrough)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // Y = 0
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        EXPECT_TRUE(line.Intersects(box));
    }

    TEST(Line2DTests, IntersectsBoundingBoxMissReturnsFalse)
    {
        const Line2D line(Vector2(0, 1), 10.0f); // Y = 10
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        EXPECT_FALSE(line.Intersects(box));
    }

    TEST(Line2DTests, IntersectsOrientedBoundingBoxPassingThrough)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // Y = 0
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_TRUE(line.Intersects(obb));
    }

    TEST(Line2DTests, IntersectsOrientedBoundingBoxMissReturnsFalse)
    {
        const Line2D line(Vector2(0, 1), 10.0f); // Y = 10
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_FALSE(line.Intersects(obb));
    }

    TEST(Line2DTests, IntersectsBoundingPolygonPassingThrough)
    {
        const Line2D line(Vector2(0, 1), 0.0f); // Y = 0
        const BoundingPolygon2D polygon =
            BoundingPolygon2D::CreateFromVertices({Vector2(-1, -1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, 1)});
        EXPECT_TRUE(line.Intersects(polygon));
    }

    TEST(Line2DTests, IntersectsBoundingPolygonMissReturnsFalse)
    {
        const Line2D line(Vector2(0, 1), 10.0f); // Y = 10
        const BoundingPolygon2D polygon =
            BoundingPolygon2D::CreateFromVertices({Vector2(-1, -1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, 1)});
        EXPECT_FALSE(line.Intersects(polygon));
    }
}
