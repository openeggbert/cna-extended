// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's LineSegment2DTest.cs (22 test methods) exercises the self-contained behavior plus
// exactly one Collision2D-dependent method (DistanceToSegment, via DistanceToSegment_Parallel).
// That one test is ported faithfully below. Every other newly-ported Distance*/Intersects overload
// (now that Collision2D exists) has no upstream test to port 1:1, so it gets a fresh true/false
// case pair instead, spot-checking the delegation rather than re-deriving Collision2D's own
// algorithm correctness (already covered exhaustively by Collision2DTests.cpp).
//
// Intersects(BoundingPolygon2D, ...) (both overloads) remains deferred -- see the blocker
// explained in LineSegment2D.hpp's header comment (BoundingPolygon2D::Contains isn't ported yet)
// -- so there are no tests for it here.
#include "CNA/Extended/LineSegment2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(LineSegment2DTests, ConstructorSetsStartAndEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.Start, Vector2(0, 0));
        EXPECT_EQ(segment.End, Vector2(10, 0));
    }

    TEST(LineSegment2DTests, DirectionIsEndMinusStart)
    {
        const LineSegment2D segment(Vector2(1, 1), Vector2(4, 5));
        EXPECT_EQ(segment.getDirectionProperty(), Vector2(3, 4));
    }

    TEST(LineSegment2DTests, MidpointIsHalfwayBetweenEndpoints)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.getMidpointProperty(), Vector2(5, 0));
    }

    TEST(LineSegment2DTests, LengthMatchesDistance)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        EXPECT_FLOAT_EQ(segment.getLengthProperty(), 5.0f);
    }

    TEST(LineSegment2DTests, LengthSquaredAvoidsSquareRoot)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        EXPECT_FLOAT_EQ(segment.getLengthSquaredProperty(), 25.0f);
    }

    TEST(LineSegment2DTests, GetBoundsEnclosesEndpoints)
    {
        const LineSegment2D segment(Vector2(5, -2), Vector2(-1, 8));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(-1, -2));
        EXPECT_EQ(bounds.Max, Vector2(5, 8));
    }

    TEST(LineSegment2DTests, GetBoundsHorizontalSegment)
    {
        const LineSegment2D segment(Vector2(2, 5), Vector2(8, 5));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(2, 5));
        EXPECT_EQ(bounds.Max, Vector2(8, 5));
    }

    TEST(LineSegment2DTests, GetBoundsVerticalSegment)
    {
        const LineSegment2D segment(Vector2(5, 2), Vector2(5, 8));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(5, 2));
        EXPECT_EQ(bounds.Max, Vector2(5, 8));
    }

    TEST(LineSegment2DTests, GetBoundsReversedEndpoints)
    {
        const LineSegment2D segment(Vector2(9, 7), Vector2(1, 2));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(1, 2));
        EXPECT_EQ(bounds.Max, Vector2(9, 7));
    }

    TEST(LineSegment2DTests, GetBoundsDegenerateSegment)
    {
        const LineSegment2D segment(Vector2(5, 5), Vector2(5, 5));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(5, 5));
        EXPECT_EQ(bounds.Max, Vector2(5, 5));
    }

    TEST(LineSegment2DTests, GetPointInterpolatesAlongSegment)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.GetPoint(0.0f), Vector2(0, 0));
        EXPECT_EQ(segment.GetPoint(0.5f), Vector2(5, 0));
        EXPECT_EQ(segment.GetPoint(1.0f), Vector2(10, 0));
    }

    TEST(LineSegment2DTests, GetPointExtrapolatesBeyondEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.GetPoint(1.5f), Vector2(15, 0));
    }

    TEST(LineSegment2DTests, ClosestPointClampsToStart)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(-5, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(0, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 0.0f);
    }

    TEST(LineSegment2DTests, ClosestPointClampsToEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(15, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(10, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 1.0f);
    }

    TEST(LineSegment2DTests, ClosestPointProjectsOntoSegment)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(5, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(5, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 0.5f);
    }

    TEST(LineSegment2DTests, DeconstructReturnsStartAndEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        Vector2 start, end;
        segment.Deconstruct(start, end);
        EXPECT_EQ(start, Vector2(0, 0));
        EXPECT_EQ(end, Vector2(10, 0));
    }

    TEST(LineSegment2DTests, EqualsAndOperators)
    {
        const LineSegment2D a(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D b(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D c(Vector2(0, 0), Vector2(5, 0));

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(LineSegment2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const LineSegment2D a(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D b(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(LineSegment2DTests, ToStringContainsStartEndAndLength)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        const std::string text = segment.ToString();
        EXPECT_NE(text.find("Start"), std::string::npos);
        EXPECT_NE(text.find("End"), std::string::npos);
        EXPECT_NE(text.find("Length"), std::string::npos);
    }

    TEST(LineSegment2DTests, DistanceToPointPerpendicularDistance)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_FLOAT_EQ(segment.DistanceToPoint(Vector2(5, 3)), 3.0f);
        EXPECT_FLOAT_EQ(segment.DistanceSquaredToPoint(Vector2(5, 3)), 9.0f);
    }

    TEST(LineSegment2DTests, DistanceToSegmentParallel)
    {
        const LineSegment2D segment1(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D segment2(Vector2(0, 5), Vector2(10, 5));

        const float distance = segment1.DistanceToSegment(segment2);

        EXPECT_NEAR(distance, 5.0f, 1e-4f);
    }

    TEST(LineSegment2DTests, DistanceSquaredToSegmentReportsClosestPoints)
    {
        const LineSegment2D segment1(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D segment2(Vector2(0, 5), Vector2(10, 5));
        float t1 = -1.0f;
        float t2 = -1.0f;
        Vector2 closest1;
        Vector2 closest2;

        const float distanceSquared = segment1.DistanceSquaredToSegment(segment2, t1, t2, closest1, closest2);

        EXPECT_NEAR(distanceSquared, 25.0f, 1e-4f);
    }

    TEST(LineSegment2DTests, IntersectsLineCrossing)
    {
        const LineSegment2D segment(Vector2(-5, 0), Vector2(5, 0));
        const Line2D line(Vector2(1, 0), 0.0f); // X = 0
        std::optional<float> distanceAlongSegment;
        std::optional<Vector2> point;
        EXPECT_TRUE(segment.Intersects(line, distanceAlongSegment, point));
        ASSERT_TRUE(distanceAlongSegment.has_value());
        EXPECT_NEAR(*distanceAlongSegment, 0.5f, 1e-4f);
    }

    TEST(LineSegment2DTests, IntersectsLineOutsideBoundsReturnsFalse)
    {
        const LineSegment2D segment(Vector2(-5, 0), Vector2(5, 0));
        const Line2D line(Vector2(1, 0), 10.0f); // X = 10
        EXPECT_FALSE(segment.Intersects(line));
    }

    TEST(LineSegment2DTests, IntersectsRayCrossing)
    {
        const LineSegment2D segment(Vector2(5, -5), Vector2(5, 5));
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        std::optional<float> distanceAlongSegment;
        std::optional<float> distanceAlongRay;
        std::optional<Vector2> point;
        EXPECT_TRUE(segment.Intersects(ray, distanceAlongSegment, distanceAlongRay, point));
        ASSERT_TRUE(distanceAlongSegment.has_value());
        ASSERT_TRUE(distanceAlongRay.has_value());
        EXPECT_NEAR(*distanceAlongSegment, 0.5f, 1e-4f);
        EXPECT_NEAR(*distanceAlongRay, 5.0f, 1e-4f);
    }

    TEST(LineSegment2DTests, IntersectsRayOutsideBoundsReturnsFalse)
    {
        const LineSegment2D segment(Vector2(5, 1), Vector2(5, 5));
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        EXPECT_FALSE(segment.Intersects(ray));
    }

    TEST(LineSegment2DTests, IntersectsLineSegmentCrossing)
    {
        const LineSegment2D a(Vector2(0, -5), Vector2(0, 5));
        const LineSegment2D b(Vector2(-5, 0), Vector2(5, 0));
        std::optional<float> t1;
        std::optional<float> t2;
        std::optional<Vector2> point;
        EXPECT_TRUE(a.Intersects(b, t1, t2, point));
        ASSERT_TRUE(point.has_value());
        EXPECT_NEAR(point->X, 0.0f, 1e-4f);
        EXPECT_NEAR(point->Y, 0.0f, 1e-4f);
    }

    TEST(LineSegment2DTests, IntersectsLineSegmentDisjointReturnsFalse)
    {
        const LineSegment2D a(Vector2(0, -5), Vector2(0, -1));
        const LineSegment2D b(Vector2(-5, 0), Vector2(5, 0));
        EXPECT_FALSE(a.Intersects(b));
    }

    TEST(LineSegment2DTests, IntersectsBoundingBoxCrossing)
    {
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(segment.Intersects(box, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        ASSERT_TRUE(tMax.has_value());
    }

    TEST(LineSegment2DTests, IntersectsBoundingBoxMissReturnsFalse)
    {
        const LineSegment2D segment(Vector2(-10, 5), Vector2(10, 5));
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        EXPECT_FALSE(segment.Intersects(box));
    }

    TEST(LineSegment2DTests, IntersectsBoundingBoxDegenerateInsideReturnsTrue)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(0, 0));
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(segment.Intersects(box, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        ASSERT_TRUE(tMax.has_value());
        EXPECT_FLOAT_EQ(*tMin, 0.0f);
        EXPECT_FLOAT_EQ(*tMax, 0.0f);
    }

    TEST(LineSegment2DTests, IntersectsBoundingCircleCrossing)
    {
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));
        const BoundingCircle2D circle(Vector2(0, 0), 2.0f);
        std::optional<float> tSegmentMin;
        std::optional<float> tSegmentMax;
        EXPECT_TRUE(segment.Intersects(circle, tSegmentMin, tSegmentMax));
        ASSERT_TRUE(tSegmentMin.has_value());
        ASSERT_TRUE(tSegmentMax.has_value());
    }

    TEST(LineSegment2DTests, IntersectsBoundingCircleMissReturnsFalse)
    {
        const LineSegment2D segment(Vector2(-10, 5), Vector2(10, 5));
        const BoundingCircle2D circle(Vector2(0, 0), 2.0f);
        EXPECT_FALSE(segment.Intersects(circle));
    }

    TEST(LineSegment2DTests, IntersectsBoundingCapsuleCrossing)
    {
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));
        const BoundingCapsule2D capsule(Vector2(-2, 0), Vector2(2, 0), 1.0f);
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(segment.Intersects(capsule, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        ASSERT_TRUE(tMax.has_value());
    }

    TEST(LineSegment2DTests, IntersectsBoundingCapsuleMissReturnsFalse)
    {
        const LineSegment2D segment(Vector2(-10, 10), Vector2(10, 10));
        const BoundingCapsule2D capsule(Vector2(-2, 0), Vector2(2, 0), 1.0f);
        EXPECT_FALSE(segment.Intersects(capsule));
    }

    TEST(LineSegment2DTests, IntersectsOrientedBoundingBoxCrossing)
    {
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(segment.Intersects(obb, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        ASSERT_TRUE(tMax.has_value());
    }

    TEST(LineSegment2DTests, IntersectsOrientedBoundingBoxMissReturnsFalse)
    {
        const LineSegment2D segment(Vector2(-10, 5), Vector2(10, 5));
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_FALSE(segment.Intersects(obb));
    }
}
