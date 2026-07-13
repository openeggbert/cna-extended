// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingCircle2DTest.cs
// (the TryGetCollision tests) and BoundingCircleTest.cs (everything else, including the
// ContainsPoint tests -- that file tests BoundingCircle2D despite its own name). Upstream has no
// Contains(shape)/Intersects test coverage for BoundingCircle2D beyond ContainsPoint, so none is
// added here beyond what's ported 1:1 below.
#include "CNA/Extended/BoundingCircle2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/CollisionResult2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

#include <cmath>
#include <numbers>

namespace CNA::Extended
{
    TEST(BoundingCircle2DTests, Constructor)
    {
        const BoundingCircle2D circle(Vector2(1, 2), 5.0f);
        EXPECT_EQ(circle.Center, Vector2(1, 2));
        EXPECT_FLOAT_EQ(circle.Radius, 5.0f);
    }

    TEST(BoundingCircle2DTests, RadiusSquared)
    {
        const BoundingCircle2D circle(Vector2::Zero, 3.0f);
        EXPECT_FLOAT_EQ(circle.getRadiusSquaredProperty(), 9.0f);
    }

    TEST(BoundingCircle2DTests, Diameter)
    {
        const BoundingCircle2D circle(Vector2::Zero, 3.0f);
        EXPECT_FLOAT_EQ(circle.getDiameterProperty(), 6.0f);
    }

    TEST(BoundingCircle2DTests, Area)
    {
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);
        EXPECT_NEAR(circle.getAreaProperty(), std::numbers::pi_v<float> * 4.0f, 1e-4f);
    }

    TEST(BoundingCircle2DTests, CreateFromPointsSinglePointHasZeroRadius)
    {
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromPoints({Vector2(3, 4)});
        EXPECT_EQ(circle.Center, Vector2(3, 4));
        EXPECT_FLOAT_EQ(circle.Radius, 0.0f);
    }

    TEST(BoundingCircle2DTests, CreateFromPointsEnclosesAllPoints)
    {
        const std::vector<Vector2> points = {Vector2(-2, 0), Vector2(2, 0), Vector2(0, 2), Vector2(0, -2)};
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromPoints(points);

        for (const Vector2& p : points)
        {
            const Vector2 d = p - circle.Center;
            const float dist = std::sqrt(d.X * d.X + d.Y * d.Y);
            EXPECT_LE(dist, circle.Radius + 1e-3f);
        }
    }

    TEST(BoundingCircle2DTests, CreateFromPointsTwoPoints)
    {
        const std::vector<Vector2> points = {Vector2(0, 0), Vector2(10, 0)};
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromPoints(points);

        EXPECT_EQ(circle.Center, Vector2(5, 0));
        EXPECT_NEAR(circle.Radius, 5.0f, 1e-4f);
    }

    TEST(BoundingCircle2DTests, CreateFromPointsMultiplePointsUpstreamSquare)
    {
        const std::vector<Vector2> points = {Vector2(0, 0), Vector2(10, 0), Vector2(0, 10), Vector2(10, 10)};
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromPoints(points);

        for (const Vector2& p : points)
        {
            EXPECT_LE(Vector2::Distance(circle.Center, p), circle.Radius + 1e-3f);
        }
    }

    TEST(BoundingCircle2DTests, CreateFromPointsThrowsOnEmpty)
    {
        EXPECT_THROW((void)BoundingCircle2D::CreateFromPoints({}), std::invalid_argument);
    }

    TEST(BoundingCircle2DTests, CreateFromBoundingCapsule2DDegenerateCapsule)
    {
        const BoundingCapsule2D capsule(Vector2(5, 5), Vector2(5, 5), 3.0f);
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromBoundingCapsule2D(capsule);

        EXPECT_EQ(circle.Center, Vector2(5, 5));
        EXPECT_NEAR(circle.Radius, 3.0f, 1e-4f);
    }

    TEST(BoundingCircle2DTests, TransformTranslation)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 5.0f);
        const Matrix matrix = Matrix::CreateTranslation(10, 20, 0);
        const BoundingCircle2D transformed = circle.Transform(matrix);

        EXPECT_EQ(transformed.Center, Vector2(10, 20));
        EXPECT_NEAR(transformed.Radius, 5.0f, 1e-4f);
    }

    TEST(BoundingCircle2DTests, TransformUniformScale)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 5.0f);
        const Matrix matrix = Matrix::CreateScale(2.0f);
        const BoundingCircle2D transformed = circle.Transform(matrix);

        EXPECT_EQ(transformed.Center, Vector2(0, 0));
        EXPECT_NEAR(transformed.Radius, 10.0f, 1e-4f);
    }

    TEST(BoundingCircle2DTests, TransformNonUniformScale)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 5.0f);
        const Matrix matrix = Matrix::CreateScale(2.0f, 3.0f, 1.0f);
        const BoundingCircle2D transformed = circle.Transform(matrix);

        EXPECT_GE(transformed.Radius, 10.0f);
    }

    TEST(BoundingCircle2DTests, TransformRotation)
    {
        const BoundingCircle2D circle(Vector2(5, 0), 3.0f);
        const Matrix matrix = Matrix::CreateRotationZ(Microsoft::Xna::Framework::MathHelper::PiOver2);
        const BoundingCircle2D transformed = circle.Transform(matrix);

        constexpr float tolerance = 1e-4f;
        EXPECT_NEAR(transformed.Center.X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Center.Y, 5.0f, tolerance);
        EXPECT_NEAR(transformed.Radius, 3.0f, tolerance);
    }

    TEST(BoundingCircle2DTests, CreateFromBoundingBox2DEnclosesBox)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(4, 4));
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromBoundingBox2D(box);

        EXPECT_EQ(circle.Center, Vector2(2, 2));
        EXPECT_NEAR(circle.Radius, std::sqrt(8.0f), 1e-4f);
    }

    TEST(BoundingCircle2DTests, CreateFromBoundingCapsule2D)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(4, 0), 1.0f);
        const BoundingCircle2D circle = BoundingCircle2D::CreateFromBoundingCapsule2D(capsule);

        EXPECT_EQ(circle.Center, Vector2(2, 0));
        EXPECT_FLOAT_EQ(circle.Radius, 3.0f); // half-length (2) + capsule radius (1)
    }

    TEST(BoundingCircle2DTests, CreateMergedOneContainsOtherReturnsLarger)
    {
        const BoundingCircle2D small(Vector2::Zero, 1.0f);
        const BoundingCircle2D big(Vector2::Zero, 5.0f);
        const BoundingCircle2D merged = BoundingCircle2D::CreateMerged(small, big);

        EXPECT_EQ(merged, big);
    }

    TEST(BoundingCircle2DTests, TranslateMovesCenter)
    {
        const BoundingCircle2D circle(Vector2(1, 1), 2.0f);
        const BoundingCircle2D translated = circle.Translate(Vector2(3, -1));

        EXPECT_EQ(translated.Center, Vector2(4, 0));
        EXPECT_FLOAT_EQ(translated.Radius, 2.0f);
    }

    TEST(BoundingCircle2DTests, DeconstructReturnsCenterAndRadius)
    {
        const BoundingCircle2D circle(Vector2(1, 2), 5.0f);
        Vector2 center;
        float radius;
        circle.Deconstruct(center, radius);

        EXPECT_EQ(center, Vector2(1, 2));
        EXPECT_FLOAT_EQ(radius, 5.0f);
    }

    TEST(BoundingCircle2DTests, ContainsPointInside)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 10.0f);
        const Vector2 point(5, 0);

        EXPECT_EQ(circle.Contains(point), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCircle2DTests, ContainsPointOnBoundary)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 10.0f);
        const Vector2 point(10, 0);

        EXPECT_EQ(circle.Contains(point), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCircle2DTests, ContainsPointOutside)
    {
        const BoundingCircle2D circle(Vector2(0, 0), 10.0f);
        const Vector2 point(15, 0);

        EXPECT_EQ(circle.Contains(point), Microsoft::Xna::Framework::ContainmentType::Disjoint);
    }

    TEST(BoundingCircle2DTests, TryGetCollisionWithOverlappingCircleReturnsMinimumTranslationVector)
    {
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);
        const BoundingCircle2D other(Vector2(3.0f, 0.0f), 2.0f);

        CollisionResult2D result;
        const bool intersects = circle.TryGetCollision(other, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(BoundingCircle2DTests, TryGetCollisionWithSeparatedCircleReturnsFalseAndNone)
    {
        const BoundingCircle2D circle(Vector2::Zero, 1.0f);
        const BoundingCircle2D other(Vector2(4.0f, 0.0f), 1.0f);

        CollisionResult2D result;
        const bool intersects = circle.TryGetCollision(other, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(Vector2::Zero, result.Normal);
        EXPECT_FLOAT_EQ(0.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2::Zero, result.MinimumTranslationVector);
    }

    TEST(BoundingCircle2DTests, TryGetCollisionWithBoxReturnsMinimumTranslationVector)
    {
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);
        const BoundingBox2D box(Vector2(1.0f, -2.0f), Vector2(5.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = circle.TryGetCollision(box, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(BoundingCircle2DTests, TryGetCollisionWithCapsuleReturnsMinimumTranslationVector)
    {
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);
        const BoundingCapsule2D capsule(Vector2(3.0f, -2.0f), Vector2(3.0f, 2.0f), 2.0f);

        CollisionResult2D result;
        const bool intersects = circle.TryGetCollision(capsule, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(BoundingCircle2DTests, TryGetCollisionWithOrientedBoxReturnsMinimumTranslationVector)
    {
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);
        const OrientedBoundingBox2D obb(Vector2(3.0f, 0.0f), Vector2::UnitX, Vector2::UnitY, Vector2(2.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = circle.TryGetCollision(obb, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(-Vector2::UnitX, result.Normal);
        EXPECT_FLOAT_EQ(1.0f, result.PenetrationDepth);
        EXPECT_EQ(Vector2(-1.0f, 0.0f), result.MinimumTranslationVector);
    }

    TEST(BoundingCircle2DTests, EqualsAndOperators)
    {
        const BoundingCircle2D a(Vector2(1, 2), 5.0f);
        const BoundingCircle2D b(Vector2(1, 2), 5.0f);
        const BoundingCircle2D c(Vector2(1, 2), 6.0f);

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(BoundingCircle2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const BoundingCircle2D a(Vector2(1, 2), 5.0f);
        const BoundingCircle2D b(Vector2(1, 2), 5.0f);

        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }
}
