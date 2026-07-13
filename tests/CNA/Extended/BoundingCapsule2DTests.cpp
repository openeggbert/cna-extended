// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingCapsule2DTest.cs.
#include "CNA/Extended/BoundingCapsule2D.hpp"

#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

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

    TEST(BoundingCapsule2DTests, CreateFromCenterAndDirectionUnnormalizedDirection)
    {
        const BoundingCapsule2D capsule = BoundingCapsule2D::CreateFromCenterAndDirection(Vector2(5, 5), Vector2(3, 4), 10.0f, 3.0f);
        EXPECT_NEAR(Vector2::Distance(capsule.PointA, capsule.PointB), 10.0f, 1e-5f);
    }

    TEST(BoundingCapsule2DTests, TransformTranslation)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 3.0f);
        const Matrix matrix = Matrix::CreateTranslation(5, 10, 0);
        const BoundingCapsule2D transformed = capsule.Transform(matrix);

        EXPECT_EQ(transformed.PointA, Vector2(5, 10));
        EXPECT_EQ(transformed.PointB, Vector2(15, 10));
        EXPECT_NEAR(transformed.Radius, 3.0f, 1e-5f);
    }

    TEST(BoundingCapsule2DTests, TransformUniformScale)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 3.0f);
        const Matrix matrix = Matrix::CreateScale(2.0f);
        const BoundingCapsule2D transformed = capsule.Transform(matrix);

        EXPECT_EQ(transformed.PointA, Vector2(0, 0));
        EXPECT_EQ(transformed.PointB, Vector2(20, 0));
        EXPECT_NEAR(transformed.Radius, 6.0f, 1e-5f);
    }

    TEST(BoundingCapsule2DTests, TransformNonUniformScale)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 3.0f);
        const Matrix matrix = Matrix::CreateScale(2.0f, 3.0f, 1.0f);
        const BoundingCapsule2D transformed = capsule.Transform(matrix);

        EXPECT_EQ(transformed.PointA, Vector2(0, 0));
        EXPECT_EQ(transformed.PointB, Vector2(20, 0));
        EXPECT_GE(transformed.Radius, 6.0f);
    }

    TEST(BoundingCapsule2DTests, TransformRotation)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 3.0f);
        const Matrix matrix = Matrix::CreateRotationZ(Microsoft::Xna::Framework::MathHelper::PiOver2);
        const BoundingCapsule2D transformed = capsule.Transform(matrix);

        constexpr float tolerance = 1e-5f;
        EXPECT_NEAR(transformed.PointA.X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.PointA.Y, 0.0f, tolerance);
        EXPECT_NEAR(transformed.PointB.X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.PointB.Y, 10.0f, tolerance);
        EXPECT_NEAR(transformed.Radius, 3.0f, tolerance);
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

    TEST(BoundingCapsule2DTests, CreateFromSegment)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        constexpr float radius = 3.0f;

        const BoundingCapsule2D capsule = BoundingCapsule2D::CreateFromSegment(segment, radius);

        EXPECT_EQ(capsule.PointA, segment.Start);
        EXPECT_EQ(capsule.PointB, segment.End);
        EXPECT_FLOAT_EQ(capsule.Radius, radius);
    }

    TEST(BoundingCapsule2DTests, CreateMergedNonOverlapping)
    {
        const BoundingCapsule2D capsule1(Vector2(0, 0), Vector2(5, 0), 2.0f);
        const BoundingCapsule2D capsule2(Vector2(20, 0), Vector2(25, 0), 2.0f);

        const BoundingCapsule2D merged = BoundingCapsule2D::CreateMerged(capsule1, capsule2);

        EXPECT_EQ(merged.Contains(capsule1), Microsoft::Xna::Framework::ContainmentType::Contains);
        EXPECT_EQ(merged.Contains(capsule2), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCapsule2DTests, CreateMergedOneContainsOther)
    {
        const BoundingCapsule2D capsule1(Vector2(0, 0), Vector2(20, 0), 5.0f);
        const BoundingCapsule2D capsule2(Vector2(8, 0), Vector2(12, 0), 2.0f);

        const BoundingCapsule2D merged = BoundingCapsule2D::CreateMerged(capsule1, capsule2);

        EXPECT_NEAR(merged.PointA.X, capsule1.PointA.X, 1e-6f);
        EXPECT_NEAR(merged.PointA.Y, capsule1.PointA.Y, 1e-6f);
        EXPECT_NEAR(merged.PointB.X, capsule1.PointB.X, 1e-6f);
        EXPECT_NEAR(merged.PointB.Y, capsule1.PointB.Y, 1e-6f);
    }

    TEST(BoundingCapsule2DTests, CreateMergedPartiallyOverlapping)
    {
        const BoundingCapsule2D capsule1(Vector2(0, 0), Vector2(10, 0), 3.0f);
        const BoundingCapsule2D capsule2(Vector2(8, 0), Vector2(18, 0), 3.0f);

        const BoundingCapsule2D merged = BoundingCapsule2D::CreateMerged(capsule1, capsule2);

        EXPECT_EQ(merged.Contains(capsule1), Microsoft::Xna::Framework::ContainmentType::Contains);
        EXPECT_EQ(merged.Contains(capsule2), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCapsule2DTests, ContainsPointInside)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 5.0f);
        const Vector2 point(5, 2);

        EXPECT_EQ(capsule.Contains(point), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCapsule2DTests, ContainsPointOnBoundary)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 5.0f);
        const Vector2 point(5, 5);

        EXPECT_EQ(capsule.Contains(point), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCapsule2DTests, ContainsPointOutside)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 5.0f);
        const Vector2 point(5, 10);

        EXPECT_EQ(capsule.Contains(point), Microsoft::Xna::Framework::ContainmentType::Disjoint);
    }

    TEST(BoundingCapsule2DTests, ContainsPointAtEndCap)
    {
        const BoundingCapsule2D capsule(Vector2(0, 0), Vector2(10, 0), 5.0f);
        const Vector2 point(-3, 4);

        EXPECT_EQ(capsule.Contains(point), Microsoft::Xna::Framework::ContainmentType::Contains);
    }

    TEST(BoundingCapsule2DTests, TryGetCollisionWithCircleReturnsReceiverMinimumTranslationVector)
    {
        const BoundingCapsule2D capsule(Vector2(3.0f, -2.0f), Vector2(3.0f, 2.0f), 2.0f);
        const BoundingCircle2D circle(Vector2::Zero, 2.0f);

        CollisionResult2D result;
        const bool intersects = capsule.TryGetCollision(circle, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(1.0f, 0.0f));
    }

    TEST(BoundingCapsule2DTests, TryGetCollisionWithSeparatedCircleReturnsFalseAndNone)
    {
        const BoundingCapsule2D capsule(Vector2(4.0f, -2.0f), Vector2(4.0f, 2.0f), 1.0f);
        const BoundingCircle2D circle(Vector2::Zero, 1.0f);

        CollisionResult2D result;
        const bool intersects = capsule.TryGetCollision(circle, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(result.Normal, CollisionResult2D::None.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, CollisionResult2D::None.PenetrationDepth);
        EXPECT_EQ(result.MinimumTranslationVector, CollisionResult2D::None.MinimumTranslationVector);
    }
}
