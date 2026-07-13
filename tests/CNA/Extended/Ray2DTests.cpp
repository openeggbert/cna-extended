// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's Ray2DTest.cs (13 test methods) exercises only the self-contained behavior: none of
// its Intersects(...) overloads are tested upstream. Those overloads are ported below (now that
// Collision2D exists) with fresh tests, since there is no upstream coverage to port 1:1 -- one
// true case and one false case per overload, spot-checking the delegation to Collision2D/Line2D
// rather than re-deriving Collision2D's own algorithm correctness (already covered exhaustively
// by Collision2DTests.cpp).
#include "CNA/Extended/Ray2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(Ray2DTests, ConstructorSetsOriginAndDirection)
    {
        const Ray2D ray(Vector2(1, 2), Vector2(1, 0));
        EXPECT_EQ(ray.Origin, Vector2(1, 2));
        EXPECT_EQ(ray.Direction, Vector2(1, 0));
    }

    TEST(Ray2DTests, CreateFromPointsProducesUnitDirectionTowardTarget)
    {
        const Ray2D ray = Ray2D::CreateFromPoints(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(ray.Origin, Vector2(0, 0));
        EXPECT_NEAR(ray.Direction.Length(), 1.0f, 1e-4f);
        EXPECT_NEAR(ray.Direction.X, 1.0f, 1e-4f);
    }

    TEST(Ray2DTests, CreateFromPointsThrowsWhenPointsCoincide)
    {
        EXPECT_THROW((void)Ray2D::CreateFromPoints(Vector2(1, 1), Vector2(1, 1)), std::invalid_argument);
    }

    TEST(Ray2DTests, GetPointInterpolatesAlongRay)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        EXPECT_EQ(ray.GetPoint(0.0f), Vector2(0, 0));
        EXPECT_EQ(ray.GetPoint(5.0f), Vector2(5, 0));
    }

    TEST(Ray2DTests, ClosestPointClampsToOriginBehindRay)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        float distanceAlongRay = -1.0f;
        const Vector2 closest = ray.ClosestPoint(Vector2(-5, 3), distanceAlongRay);
        EXPECT_EQ(closest, Vector2(0, 0));
        EXPECT_FLOAT_EQ(distanceAlongRay, 0.0f);
    }

    TEST(Ray2DTests, ClosestPointProjectsOntoRay)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        float distanceAlongRay = -1.0f;
        const Vector2 closest = ray.ClosestPoint(Vector2(5, 3), distanceAlongRay);
        EXPECT_EQ(closest, Vector2(5, 0));
        EXPECT_FLOAT_EQ(distanceAlongRay, 5.0f);
    }

    TEST(Ray2DTests, DistanceToPointMatchesPerpendicularDistance)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        EXPECT_FLOAT_EQ(ray.DistanceToPoint(Vector2(5, 3)), 3.0f);
        EXPECT_FLOAT_EQ(ray.DistanceSquaredToPoint(Vector2(5, 3)), 9.0f);
    }

    TEST(Ray2DTests, NormalizeProducesUnitDirection)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(3, 4));
        const Ray2D normalized = Ray2D::Normalize(ray);
        EXPECT_NEAR(normalized.Direction.Length(), 1.0f, 1e-4f);
        EXPECT_EQ(normalized.Origin, Vector2(0, 0));
    }

    TEST(Ray2DTests, InstanceNormalizeMutatesInPlace)
    {
        Ray2D ray(Vector2(0, 0), Vector2(3, 4));
        ray.Normalize();
        EXPECT_NEAR(ray.Direction.Length(), 1.0f, 1e-4f);
    }

    TEST(Ray2DTests, DeconstructReturnsOriginAndDirection)
    {
        const Ray2D ray(Vector2(1, 2), Vector2(1, 0));
        Vector2 origin, direction;
        ray.Deconstruct(origin, direction);
        EXPECT_EQ(origin, Vector2(1, 2));
        EXPECT_EQ(direction, Vector2(1, 0));
    }

    TEST(Ray2DTests, EqualsAndOperators)
    {
        const Ray2D a(Vector2(0, 0), Vector2(1, 0));
        const Ray2D b(Vector2(0, 0), Vector2(1, 0));
        const Ray2D c(Vector2(0, 0), Vector2(0, 1));

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(Ray2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const Ray2D a(Vector2(0, 0), Vector2(1, 0));
        const Ray2D b(Vector2(0, 0), Vector2(1, 0));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(Ray2DTests, ToStringContainsOriginAndDirection)
    {
        const Ray2D ray(Vector2(1, 2), Vector2(1, 0));
        const std::string text = ray.ToString();
        EXPECT_NE(text.find("Origin"), std::string::npos);
        EXPECT_NE(text.find("Direction"), std::string::npos);
    }

    TEST(Ray2DTests, IntersectsLineHitsInFrontOfRay)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        const Line2D line(Vector2(1, 0), 5.0f); // X = 5
        std::optional<float> distanceAlongRay;
        std::optional<Vector2> point;
        EXPECT_TRUE(ray.Intersects(line, distanceAlongRay, point));
        ASSERT_TRUE(distanceAlongRay.has_value());
        EXPECT_NEAR(*distanceAlongRay, 5.0f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsLineBehindRayReturnsFalse)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        const Line2D line(Vector2(1, 0), -5.0f); // X = -5
        EXPECT_FALSE(ray.Intersects(line));
    }

    TEST(Ray2DTests, IntersectsRayCrossingInFrontOfBoth)
    {
        const Ray2D a(Vector2(0, 0), Vector2(1, 0));
        const Ray2D b(Vector2(5, -5), Vector2(0, 1));
        std::optional<float> t1;
        std::optional<float> t2;
        std::optional<Vector2> point;
        EXPECT_TRUE(a.Intersects(b, t1, t2, point));
        ASSERT_TRUE(point.has_value());
        EXPECT_NEAR(point->X, 5.0f, 1e-4f);
        EXPECT_NEAR(point->Y, 0.0f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsRayParallelReturnsFalse)
    {
        const Ray2D a(Vector2(0, 0), Vector2(1, 0));
        const Ray2D b(Vector2(0, 5), Vector2(1, 0));
        EXPECT_FALSE(a.Intersects(b));
    }

    TEST(Ray2DTests, IntersectsLineSegmentCrossing)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        const LineSegment2D segment(Vector2(5, -5), Vector2(5, 5));
        std::optional<float> distanceAlongRay;
        std::optional<float> distanceAlongSegment;
        std::optional<Vector2> point;
        EXPECT_TRUE(ray.Intersects(segment, distanceAlongRay, distanceAlongSegment, point));
        ASSERT_TRUE(distanceAlongSegment.has_value());
        EXPECT_NEAR(*distanceAlongSegment, 0.5f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsLineSegmentOutsideBoundsReturnsFalse)
    {
        const Ray2D ray(Vector2(0, 0), Vector2(1, 0));
        const LineSegment2D segment(Vector2(5, 1), Vector2(5, 5));
        EXPECT_FALSE(ray.Intersects(segment));
    }

    TEST(Ray2DTests, IntersectsBoundingBoxFromOutside)
    {
        const Ray2D ray(Vector2(-10, 0), Vector2(1, 0));
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(ray.Intersects(box, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        ASSERT_TRUE(tMax.has_value());
        EXPECT_NEAR(*tMin, 9.0f, 1e-4f);
        EXPECT_NEAR(*tMax, 11.0f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsBoundingBoxMissReturnsFalse)
    {
        const Ray2D ray(Vector2(-10, 5), Vector2(1, 0));
        const BoundingBox2D box(Vector2(-1, -1), Vector2(1, 1));
        EXPECT_FALSE(ray.Intersects(box));
    }

    TEST(Ray2DTests, IntersectsBoundingCircleFromOutside)
    {
        const Ray2D ray(Vector2(-10, 0), Vector2(1, 0));
        const BoundingCircle2D circle(Vector2(0, 0), 2.0f);
        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_TRUE(ray.Intersects(circle, tMin, tMax));
        ASSERT_TRUE(tMin.has_value());
        EXPECT_NEAR(*tMin, 8.0f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsBoundingCircleMissReturnsFalse)
    {
        const Ray2D ray(Vector2(-10, 5), Vector2(1, 0));
        const BoundingCircle2D circle(Vector2(0, 0), 2.0f);
        EXPECT_FALSE(ray.Intersects(circle));
    }

    TEST(Ray2DTests, IntersectsBoundingCapsuleFromOutside)
    {
        const Ray2D ray(Vector2(-10, 0), Vector2(1, 0));
        const BoundingCapsule2D capsule(Vector2(-2, 0), Vector2(2, 0), 1.0f);
        EXPECT_TRUE(ray.Intersects(capsule));
    }

    TEST(Ray2DTests, IntersectsBoundingCapsuleMissReturnsFalse)
    {
        const Ray2D ray(Vector2(-10, 10), Vector2(1, 0));
        const BoundingCapsule2D capsule(Vector2(-2, 0), Vector2(2, 0), 1.0f);
        EXPECT_FALSE(ray.Intersects(capsule));
    }

    TEST(Ray2DTests, IntersectsOrientedBoundingBoxFromOutside)
    {
        const Ray2D ray(Vector2(-10, 0), Vector2(1, 0));
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_TRUE(ray.Intersects(obb));
    }

    TEST(Ray2DTests, IntersectsOrientedBoundingBoxMissReturnsFalse)
    {
        const Ray2D ray(Vector2(-10, 5), Vector2(1, 0));
        const OrientedBoundingBox2D obb(Vector2(0, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_FALSE(ray.Intersects(obb));
    }

    TEST(Ray2DTests, IntersectsBoundingPolygonFromOutside)
    {
        const Ray2D ray(Vector2(-10, 0), Vector2(1, 0));
        const BoundingPolygon2D polygon =
            BoundingPolygon2D::CreateFromVertices({Vector2(-1, -1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, 1)});
        std::optional<float> tMin;
        std::optional<float> tMax;
        std::optional<Vector2> point;
        EXPECT_TRUE(ray.Intersects(polygon, tMin, tMax, point));
        ASSERT_TRUE(tMin.has_value());
        EXPECT_NEAR(*tMin, 9.0f, 1e-4f);
    }

    TEST(Ray2DTests, IntersectsBoundingPolygonMissReturnsFalse)
    {
        const Ray2D ray(Vector2(-10, 5), Vector2(1, 0));
        const BoundingPolygon2D polygon =
            BoundingPolygon2D::CreateFromVertices({Vector2(-1, -1), Vector2(1, -1), Vector2(1, 1), Vector2(-1, 1)});
        EXPECT_FALSE(ray.Intersects(polygon));
    }
}
