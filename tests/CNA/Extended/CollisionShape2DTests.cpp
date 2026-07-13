// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Fresh test coverage for CollisionShape2D -- no upstream CollisionShape2DTest.cs exists (confirmed:
// no test file for this type anywhere under MonoGame.Extended.Tests). Follows the "spot-check pair
// per delegation branch" convention used throughout this session's Collision2D follow-up work:
// one true and one false case per dispatch branch, not an exhaustive re-derivation of each bounding
// volume type's own intersection-algorithm correctness (already covered by their own test files and
// by Collision2DTests.cpp). Also covers the None-shape default-false behavior and the legacy
// (pre-CollisionResult2D) penetration-vector helper's 4 supported pairs plus its "unsupported pair"
// fallback.
#include "CNA/Extended/CollisionShape2D.hpp"

#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    namespace
    {
        BoundingBox2D MakeBox(float minX, float minY, float maxX, float maxY)
        {
            return BoundingBox2D(Vector2(minX, minY), Vector2(maxX, maxY));
        }

        BoundingPolygon2D MakeSquarePolygon(float cx, float cy, float halfExtent)
        {
            return BoundingPolygon2D(std::vector<Vector2>{
                Vector2(cx - halfExtent, cy - halfExtent), Vector2(cx + halfExtent, cy - halfExtent),
                Vector2(cx + halfExtent, cy + halfExtent), Vector2(cx - halfExtent, cy + halfExtent)});
        }
    }

    TEST(CollisionShape2DTests, DefaultConstructedIsNoneAndHasZeroBounds)
    {
        const CollisionShape2D shape;
        EXPECT_EQ(shape.getBoundingBoxProperty().Min, Vector2::Zero);
        EXPECT_EQ(shape.getBoundingBoxProperty().Max, Vector2::Zero);
    }

    TEST(CollisionShape2DTests, NoneShapeNeverIntersectsAnything)
    {
        const CollisionShape2D none;
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));

        EXPECT_FALSE(none.Intersects(box));
        EXPECT_FALSE(box.Intersects(none));
        EXPECT_FALSE(none.Intersects(none));

        const Ray2D ray(Vector2(-5, 5), Vector2::UnitX);
        EXPECT_FALSE(none.Intersects(ray));

        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_FALSE(none.Intersects(ray, tMin, tMax));
        EXPECT_EQ(tMin.value_or(-1.0f), 0.0f);
        EXPECT_EQ(tMax.value_or(-1.0f), 0.0f);

        const Line2D line = Line2D::CreateFromPointAndDirection(Vector2(-5, 5), Vector2::UnitX);
        EXPECT_FALSE(none.Intersects(line));

        const LineSegment2D segment(Vector2(-5, 5), Vector2(5, 5));
        EXPECT_FALSE(none.Intersects(segment));
        EXPECT_FALSE(none.Intersects(segment, tMin, tMax));

        CollisionResult2D result;
        EXPECT_FALSE(none.TryGetCollision(box, result));
        EXPECT_FALSE(result.Intersects);
    }

    TEST(CollisionShape2DTests, BoundingBoxPropertyReflectsEachKind)
    {
        const CollisionShape2D box(MakeBox(1, 2, 10, 20));
        EXPECT_EQ(box.getBoundingBoxProperty().Min, Vector2(1, 2));
        EXPECT_EQ(box.getBoundingBoxProperty().Max, Vector2(10, 20));

        const CollisionShape2D circle(BoundingCircle2D(Vector2(5, 5), 2.0f));
        EXPECT_EQ(circle.getBoundingBoxProperty().Min, Vector2(3, 3));
        EXPECT_EQ(circle.getBoundingBoxProperty().Max, Vector2(7, 7));

        const CollisionShape2D capsule(BoundingCapsule2D(Vector2(0, 0), Vector2(10, 0), 2.0f));
        EXPECT_EQ(capsule.getBoundingBoxProperty().Min, Vector2(-2, -2));
        EXPECT_EQ(capsule.getBoundingBoxProperty().Max, Vector2(12, 2));

        const CollisionShape2D obb(OrientedBoundingBox2D(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1)));
        EXPECT_EQ(obb.getBoundingBoxProperty().Min, Vector2(-1, -1));
        EXPECT_EQ(obb.getBoundingBoxProperty().Max, Vector2(1, 1));

        const CollisionShape2D polygon(MakeSquarePolygon(0, 0, 1));
        EXPECT_EQ(polygon.getBoundingBoxProperty().Min, Vector2(-1, -1));
        EXPECT_EQ(polygon.getBoundingBoxProperty().Max, Vector2(1, 1));
    }

    TEST(CollisionShape2DTests, IntersectsBoxBoxOverlapping)
    {
        const CollisionShape2D a(MakeBox(0, 0, 10, 10));
        const CollisionShape2D b(MakeBox(5, 5, 15, 15));
        EXPECT_TRUE(a.Intersects(b));
    }

    TEST(CollisionShape2DTests, IntersectsBoxBoxSeparatedReturnsFalse)
    {
        const CollisionShape2D a(MakeBox(0, 0, 10, 10));
        const CollisionShape2D b(MakeBox(100, 100, 110, 110));
        EXPECT_FALSE(a.Intersects(b));
    }

    TEST(CollisionShape2DTests, IntersectsBoxCircleOverlapping)
    {
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));
        const CollisionShape2D circle(BoundingCircle2D(Vector2(10, 5), 2.0f));
        EXPECT_TRUE(box.Intersects(circle));
        EXPECT_TRUE(circle.Intersects(box));
    }

    TEST(CollisionShape2DTests, IntersectsCircleCircleOverlapping)
    {
        const CollisionShape2D a(BoundingCircle2D(Vector2(0, 0), 5.0f));
        const CollisionShape2D b(BoundingCircle2D(Vector2(8, 0), 5.0f));
        EXPECT_TRUE(a.Intersects(b));
    }

    TEST(CollisionShape2DTests, IntersectsObbObbOverlapping)
    {
        const CollisionShape2D a(OrientedBoundingBox2D(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1)));
        const CollisionShape2D b(OrientedBoundingBox2D(Vector2(1.5f, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1)));
        EXPECT_TRUE(a.Intersects(b));
    }

    TEST(CollisionShape2DTests, IntersectsCapsulePolygonOverlapping)
    {
        const CollisionShape2D capsule(BoundingCapsule2D(Vector2(-10, 0), Vector2(10, 0), 0.5f));
        const CollisionShape2D polygon(MakeSquarePolygon(0, 0, 1));
        EXPECT_TRUE(capsule.Intersects(polygon));
        EXPECT_TRUE(polygon.Intersects(capsule));
    }

    TEST(CollisionShape2DTests, IntersectsPolygonPolygonSeparatedReturnsFalse)
    {
        const CollisionShape2D a(MakeSquarePolygon(0, 0, 1));
        const CollisionShape2D b(MakeSquarePolygon(100, 100, 1));
        EXPECT_FALSE(a.Intersects(b));
    }

    TEST(CollisionShape2DTests, IntersectsRayHitsBox)
    {
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));
        const Ray2D ray(Vector2(-5, 5), Vector2::UnitX);
        EXPECT_TRUE(box.Intersects(ray));

        std::optional<float> tMin;
        std::optional<float> tMax;
        ASSERT_TRUE(box.Intersects(ray, tMin, tMax));
        EXPECT_EQ(tMin.value_or(-1.0f), 5.0f);
        EXPECT_EQ(tMax.value_or(-1.0f), 15.0f);
    }

    TEST(CollisionShape2DTests, IntersectsRayMissesBoxReturnsFalse)
    {
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));
        const Ray2D ray(Vector2(-5, 50), Vector2::UnitX);
        EXPECT_FALSE(box.Intersects(ray));
    }

    TEST(CollisionShape2DTests, IntersectsRayWithOutParamsExcludesPolygonKind)
    {
        const CollisionShape2D polygon(MakeSquarePolygon(0, 0, 1));
        const Ray2D ray(Vector2(-5, 0), Vector2::UnitX);

        EXPECT_TRUE(polygon.Intersects(ray));

        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_FALSE(polygon.Intersects(ray, tMin, tMax));
    }

    TEST(CollisionShape2DTests, IntersectsLineHitsCircle)
    {
        const CollisionShape2D circle(BoundingCircle2D(Vector2(5, 5), 2.0f));
        const Line2D line = Line2D::CreateFromPointAndDirection(Vector2(-100, 5), Vector2::UnitX);
        EXPECT_TRUE(circle.Intersects(line));
    }

    TEST(CollisionShape2DTests, IntersectsLineMissesCapsuleReturnsFalse)
    {
        const CollisionShape2D capsule(BoundingCapsule2D(Vector2(0, 0), Vector2(10, 0), 1.0f));
        const Line2D line = Line2D::CreateFromPointAndDirection(Vector2(-100, 50), Vector2::UnitX);
        EXPECT_FALSE(capsule.Intersects(line));
    }

    TEST(CollisionShape2DTests, IntersectsLineSegmentHitsObb)
    {
        const CollisionShape2D obb(OrientedBoundingBox2D(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1)));
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));

        EXPECT_TRUE(obb.Intersects(segment));

        std::optional<float> tMin;
        std::optional<float> tMax;
        ASSERT_TRUE(obb.Intersects(segment, tMin, tMax));
        EXPECT_TRUE(tMin.has_value());
        EXPECT_TRUE(tMax.has_value());
    }

    TEST(CollisionShape2DTests, IntersectsLineSegmentWithOutParamsExcludesPolygonKind)
    {
        const CollisionShape2D polygon(MakeSquarePolygon(0, 0, 1));
        const LineSegment2D segment(Vector2(-10, 0), Vector2(10, 0));

        EXPECT_TRUE(polygon.Intersects(segment));

        std::optional<float> tMin;
        std::optional<float> tMax;
        EXPECT_FALSE(polygon.Intersects(segment, tMin, tMax));
    }

    TEST(CollisionShape2DTests, IntersectsLineSegmentMissesReturnsFalse)
    {
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));
        const LineSegment2D segment(Vector2(-10, 50), Vector2(50, 50));
        EXPECT_FALSE(box.Intersects(segment));
    }

    TEST(CollisionShape2DTests, TryGetCollisionBoxBoxReturnsMinimumTranslationVector)
    {
        const CollisionShape2D a(MakeBox(0, 0, 10, 10));
        const CollisionShape2D b(MakeBox(8, 0, 18, 10));

        CollisionResult2D result;
        ASSERT_TRUE(a.TryGetCollision(b, result));
        EXPECT_TRUE(result.Intersects);
    }

    TEST(CollisionShape2DTests, TryGetCollisionCircleCapsuleReturnsCollision)
    {
        const CollisionShape2D circle(BoundingCircle2D(Vector2(0, 0), 3.0f));
        const CollisionShape2D capsule(BoundingCapsule2D(Vector2(2, -10), Vector2(2, 10), 1.0f));

        CollisionResult2D result;
        EXPECT_TRUE(circle.TryGetCollision(capsule, result));
    }

    TEST(CollisionShape2DTests, TryGetCollisionUnsupportedPairReturnsFalseAndNone)
    {
        // Upstream defines no Capsule/Capsule TryGetCollision overload (Collision2D has no
        // TryGetCollisionCapsuleCapsule function to wrap) -- correctly falls through to the
        // default case, even though the two capsules genuinely overlap.
        const CollisionShape2D a(BoundingCapsule2D(Vector2(-10, 0), Vector2(10, 0), 2.0f));
        const CollisionShape2D b(BoundingCapsule2D(Vector2(-10, 1), Vector2(10, 1), 2.0f));

        CollisionResult2D result;
        EXPECT_FALSE(a.TryGetCollision(b, result));
        EXPECT_FALSE(result.Intersects);
    }

    TEST(CollisionShape2DTests, TryGetCollisionSeparatedReturnsFalseAndNone)
    {
        const CollisionShape2D a(MakeBox(0, 0, 10, 10));
        const CollisionShape2D b(MakeBox(100, 100, 110, 110));

        CollisionResult2D result;
        EXPECT_FALSE(a.TryGetCollision(b, result));
        EXPECT_FALSE(result.Intersects);
    }

    TEST(CollisionShape2DTests, TryGetLegacyPenetrationVectorCircleCircle)
    {
        const CollisionShape2D a(BoundingCircle2D(Vector2(0, 0), 5.0f));
        const CollisionShape2D b(BoundingCircle2D(Vector2(8, 0), 5.0f));

        Vector2 penetrationVector;
        EXPECT_TRUE(a.TryGetLegacyPenetrationVector(b, penetrationVector));
        EXPECT_EQ(penetrationVector, Vector2(2, 0));
    }

    TEST(CollisionShape2DTests, TryGetLegacyPenetrationVectorCircleBoxAndBoxCircleAreOpposite)
    {
        const CollisionShape2D circle(BoundingCircle2D(Vector2(11, 5), 2.0f));
        const CollisionShape2D box(MakeBox(0, 0, 10, 10));

        Vector2 circleFirst;
        Vector2 boxFirst;
        ASSERT_TRUE(circle.TryGetLegacyPenetrationVector(box, circleFirst));
        ASSERT_TRUE(box.TryGetLegacyPenetrationVector(circle, boxFirst));
        EXPECT_EQ(circleFirst, -boxFirst);
    }

    TEST(CollisionShape2DTests, TryGetLegacyPenetrationVectorBoxBox)
    {
        const CollisionShape2D a(MakeBox(0, 0, 10, 10));
        const CollisionShape2D b(MakeBox(8, 0, 18, 10));

        Vector2 penetrationVector;
        ASSERT_TRUE(a.TryGetLegacyPenetrationVector(b, penetrationVector));
        EXPECT_EQ(penetrationVector, Vector2(2, 0));
    }

    TEST(CollisionShape2DTests, TryGetLegacyPenetrationVectorUnsupportedPairReturnsFalseAndZero)
    {
        const CollisionShape2D a(BoundingCircle2D(Vector2(0, 0), 5.0f));
        const CollisionShape2D b(MakeSquarePolygon(0, 0, 1));

        Vector2 penetrationVector;
        EXPECT_FALSE(a.TryGetLegacyPenetrationVector(b, penetrationVector));
        EXPECT_EQ(penetrationVector, Vector2::Zero);
    }
}
