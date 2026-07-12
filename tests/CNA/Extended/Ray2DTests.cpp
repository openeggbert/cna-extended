// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's Ray2DTest.cs (13 test methods) exercises both self-contained behavior and the
// Collision2D-dependent Intersects overloads deferred in Ray2D.hpp. These tests cover what's
// actually ported: construction, CreateFromPoints, GetPoint, ClosestPoint,
// DistanceSquaredToPoint/DistanceToPoint, Normalize, Deconstruct, Equals/GetHashCode/ToString.
#include "CNA/Extended/Ray2D.hpp"

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
}
