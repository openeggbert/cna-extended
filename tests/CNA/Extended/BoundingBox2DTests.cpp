// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingBox2DTest.cs.
// The ContainsPoint tests are upstream's own "delegation spot check" subset (it does not
// exhaustively re-test every Collision2D::Contains* overload from every bounding-volume type's
// test file, and neither do we); the TryGetCollision tests cover all 4 landed overloads
// (self, circle, obb, polygon). Upstream has no dedicated test methods for the plain
// Intersects(...) overloads or the 5 remaining Contains(...) overloads (self, circle, obb,
// capsule, polygon) beyond the point case, so none are added here either -- this mirrors
// upstream's own test coverage, not a gap in this port.
#include "CNA/Extended/BoundingBox2D.hpp"

#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/CollisionResult2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(BoundingBox2DTests, Constructor)
    {
        const Vector2 min(1, 2);
        const Vector2 max(10, 20);
        const BoundingBox2D box(min, max);

        EXPECT_EQ(box.Min, min);
        EXPECT_EQ(box.Max, max);
    }

    TEST(BoundingBox2DTests, CenterReturnsMiddlePoint)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_EQ(box.getCenterProperty(), Vector2(5, 10));
    }

    TEST(BoundingBox2DTests, SizeReturnsWidthAndHeight)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_EQ(box.getSizeProperty(), Vector2(10, 20));
    }

    TEST(BoundingBox2DTests, HalfExtentsReturnsHalfSize)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_EQ(box.getHalfExtentsProperty(), Vector2(5, 10));
    }

    TEST(BoundingBox2DTests, WidthReturnsHorizontalExtent)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_FLOAT_EQ(box.getWidthProperty(), 10.0f);
    }

    TEST(BoundingBox2DTests, HeightReturnsVerticalExtent)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_FLOAT_EQ(box.getHeightProperty(), 20.0f);
    }

    TEST(BoundingBox2DTests, AreaReturnsWidthTimesHeight)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        EXPECT_FLOAT_EQ(box.getAreaProperty(), 200.0f);
    }

    TEST(BoundingBox2DTests, CreateFromMinMax)
    {
        const Vector2 min(1, 2);
        const Vector2 max(10, 20);
        const BoundingBox2D box = BoundingBox2D::CreateFromMinMax(min, max);

        EXPECT_EQ(box.Min, min);
        EXPECT_EQ(box.Max, max);
    }

    TEST(BoundingBox2DTests, CreateFromCenterAndExtents)
    {
        const BoundingBox2D box = BoundingBox2D::CreateFromCenterAndExtents(Vector2(5, 10), Vector2(5, 10));
        EXPECT_EQ(box.Min, Vector2(0, 0));
        EXPECT_EQ(box.Max, Vector2(10, 20));
    }

    TEST(BoundingBox2DTests, CreateFromPositionAndSize)
    {
        const BoundingBox2D box = BoundingBox2D::CreateFromPositionAndSize(Vector2(1, 2), Vector2(9, 18));
        EXPECT_EQ(box.Min, Vector2(1, 2));
        EXPECT_EQ(box.Max, Vector2(10, 20));
    }

    TEST(BoundingBox2DTests, CreateFromPointsEnclosesAllPoints)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(-1, 7), Vector2(5, -2)};
        const BoundingBox2D box = BoundingBox2D::CreateFromPoints(points);

        EXPECT_EQ(box.Min, Vector2(-1, -2));
        EXPECT_EQ(box.Max, Vector2(5, 7));
    }

    TEST(BoundingBox2DTests, CreateFromPointsThrowsOnEmpty)
    {
        EXPECT_THROW((void)BoundingBox2D::CreateFromPoints({}), std::invalid_argument);
    }

    TEST(BoundingBox2DTests, CreateMergedEnclosesBoth)
    {
        const BoundingBox2D a(Vector2(0, 0), Vector2(5, 5));
        const BoundingBox2D b(Vector2(3, 3), Vector2(10, 10));
        const BoundingBox2D merged = BoundingBox2D::CreateMerged(a, b);

        EXPECT_EQ(merged.Min, Vector2(0, 0));
        EXPECT_EQ(merged.Max, Vector2(10, 10));
    }

    TEST(BoundingBox2DTests, CreateMergedOneBoxContainsOther)
    {
        const BoundingBox2D box1(Vector2(0, 0), Vector2(20, 20));
        const BoundingBox2D box2(Vector2(5, 5), Vector2(15, 15));
        const BoundingBox2D merged = BoundingBox2D::CreateMerged(box1, box2);

        EXPECT_EQ(merged.Min, box1.Min);
        EXPECT_EQ(merged.Max, box1.Max);
    }

    TEST(BoundingBox2DTests, GetCornersReturnsFourCornersCounterClockwise)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        const std::vector<Vector2> corners = box.GetCorners();

        ASSERT_EQ(corners.size(), 4u);
        EXPECT_EQ(corners[0], Vector2(0, 0));
        EXPECT_EQ(corners[1], Vector2(10, 0));
        EXPECT_EQ(corners[2], Vector2(10, 20));
        EXPECT_EQ(corners[3], Vector2(0, 20));
    }

    TEST(BoundingBox2DTests, GetCornersOutParamMatchesReturnValue)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        Vector2 corners[4];
        box.GetCorners(corners);

        const std::vector<Vector2> expected = box.GetCorners();
        for (int i = 0; i < 4; i++)
        {
            EXPECT_EQ(corners[i], expected[static_cast<std::size_t>(i)]);
        }
    }

    TEST(BoundingBox2DTests, TransformTranslation)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 10));
        const Matrix matrix = Matrix::CreateTranslation(5, 10, 0);
        const BoundingBox2D transformed = box.Transform(matrix);

        EXPECT_EQ(transformed.Min, Vector2(5, 10));
        EXPECT_EQ(transformed.Max, Vector2(15, 20));
    }

    TEST(BoundingBox2DTests, TransformScale)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 10));
        const Matrix matrix = Matrix::CreateScale(2.0f);
        const BoundingBox2D transformed = box.Transform(matrix);

        EXPECT_EQ(transformed.Min, Vector2(0, 0));
        EXPECT_EQ(transformed.Max, Vector2(20, 20));
    }

    TEST(BoundingBox2DTests, TransformRotation)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 0));
        const Matrix matrix = Matrix::CreateRotationZ(Microsoft::Xna::Framework::MathHelper::PiOver2);
        const BoundingBox2D transformed = box.Transform(matrix);

        constexpr float tolerance = 1e-5f;
        EXPECT_NEAR(transformed.Min.X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Min.Y, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Max.X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Max.Y, 10.0f, tolerance);
    }

    TEST(BoundingBox2DTests, TranslateMovesBothCorners)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        const BoundingBox2D translated = box.Translate(Vector2(5, -5));

        EXPECT_EQ(translated.Min, Vector2(5, -5));
        EXPECT_EQ(translated.Max, Vector2(15, 15));
    }

    TEST(BoundingBox2DTests, ContainsPointInside)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 10));
        const Vector2 point(5, 5);

        const ContainmentType result = box.Contains(point);

        EXPECT_EQ(result, ContainmentType::Contains);
    }

    TEST(BoundingBox2DTests, ContainsPointOnBoundary)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 10));
        const Vector2 point(10, 5);

        const ContainmentType result = box.Contains(point);

        EXPECT_EQ(result, ContainmentType::Contains);
    }

    TEST(BoundingBox2DTests, ContainsPointOutside)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 10));
        const Vector2 point(15, 5);

        const ContainmentType result = box.Contains(point);

        EXPECT_EQ(result, ContainmentType::Disjoint);
    }

    TEST(BoundingBox2DTests, TryGetCollisionWithOverlappingBoxReturnsReceiverMinimumTranslationVector)
    {
        const BoundingBox2D box(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));
        const BoundingBox2D other(Vector2(1.0f, -2.0f), Vector2(5.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = box.TryGetCollision(other, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, -Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(-1.0f, 0.0f));
    }

    TEST(BoundingBox2DTests, TryGetCollisionWithSeparatedBoxReturnsFalseAndNone)
    {
        const BoundingBox2D box(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
        const BoundingBox2D other(Vector2(4.0f, -1.0f), Vector2(6.0f, 1.0f));

        CollisionResult2D result;
        const bool intersects = box.TryGetCollision(other, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(result.Normal, CollisionResult2D::None.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, CollisionResult2D::None.PenetrationDepth);
        EXPECT_EQ(result.MinimumTranslationVector, CollisionResult2D::None.MinimumTranslationVector);
    }

    TEST(BoundingBox2DTests, TryGetCollisionWithCircleReturnsReceiverMinimumTranslationVector)
    {
        const BoundingBox2D box(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));
        const BoundingCircle2D circle(Vector2(3.0f, 0.0f), 2.0f);

        CollisionResult2D result;
        const bool intersects = box.TryGetCollision(circle, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, -Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(-1.0f, 0.0f));
    }

    TEST(BoundingBox2DTests, TryGetCollisionWithOrientedBoxReturnsReceiverMinimumTranslationVector)
    {
        const BoundingBox2D box(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));
        const OrientedBoundingBox2D obb(Vector2(3.0f, 0.0f), Vector2::UnitX, Vector2::UnitY, Vector2(2.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = box.TryGetCollision(obb, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, -Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(-1.0f, 0.0f));
    }

    TEST(BoundingBox2DTests, TryGetCollisionWithPolygonReturnsReceiverMinimumTranslationVector)
    {
        const BoundingBox2D box(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));
        const std::vector<Vector2> vertices = {
            Vector2(1.0f, -2.0f),
            Vector2(5.0f, -2.0f),
            Vector2(5.0f, 2.0f),
            Vector2(1.0f, 2.0f),
        };
        const std::vector<Vector2> normals = {
            -Vector2::UnitY,
            Vector2::UnitX,
            Vector2::UnitY,
            -Vector2::UnitX,
        };
        const BoundingPolygon2D polygon(vertices, normals);

        CollisionResult2D result;
        const bool intersects = box.TryGetCollision(polygon, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, -Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(-1.0f, 0.0f));
    }

    TEST(BoundingBox2DTests, DeconstructReturnsMinAndMax)
    {
        const BoundingBox2D box(Vector2(1, 2), Vector2(10, 20));
        Vector2 min, max;
        box.Deconstruct(min, max);

        EXPECT_EQ(min, Vector2(1, 2));
        EXPECT_EQ(max, Vector2(10, 20));
    }

    TEST(BoundingBox2DTests, EqualsAndOperators)
    {
        const BoundingBox2D a(Vector2(0, 0), Vector2(10, 20));
        const BoundingBox2D b(Vector2(0, 0), Vector2(10, 20));
        const BoundingBox2D c(Vector2(1, 0), Vector2(10, 20));

        EXPECT_TRUE(a.Equals(b));
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a == c);
        EXPECT_TRUE(a != c);
    }

    TEST(BoundingBox2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const BoundingBox2D a(Vector2(0, 0), Vector2(10, 20));
        const BoundingBox2D b(Vector2(0, 0), Vector2(10, 20));

        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(BoundingBox2DTests, ToStringContainsMinAndMax)
    {
        const BoundingBox2D box(Vector2(1, 2), Vector2(10, 20));
        const std::string text = box.ToString();

        EXPECT_NE(text.find("Min"), std::string::npos);
        EXPECT_NE(text.find("Max"), std::string::npos);
    }
}
