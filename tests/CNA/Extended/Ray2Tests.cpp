// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Ray2 (Math/Ray2.cs) -- it has zero call sites and zero test
// coverage in upstream itself (see Ray2.hpp's header comment). Fresh tests below.
#include "CNA/Extended/Ray2.hpp"

#include "CNA/Extended/BoundingRectangle.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(Ray2Tests, ConstructorSetsPositionAndDirection)
    {
        const Ray2 ray(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        EXPECT_FLOAT_EQ(ray.Position.X, 1.0f);
        EXPECT_FLOAT_EQ(ray.Position.Y, 2.0f);
        EXPECT_FLOAT_EQ(ray.Direction.X, 3.0f);
        EXPECT_FLOAT_EQ(ray.Direction.Y, 4.0f);
    }

    TEST(Ray2Tests, EqualsAndOperators)
    {
        const Ray2 a(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        const Ray2 b(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        const Ray2 c(Vector2(9.0f, 9.0f), Vector2(3.0f, 4.0f));

        EXPECT_TRUE(a.Equals(b));
        EXPECT_TRUE(a == b);
        EXPECT_FALSE(a.Equals(c));
        EXPECT_TRUE(a != c);
    }

    TEST(Ray2Tests, GetHashCodeMatchesForEqualRays)
    {
        const Ray2 a(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        const Ray2 b(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(Ray2Tests, ToStringContainsPositionAndDirection)
    {
        const Ray2 ray(Vector2(1.0f, 2.0f), Vector2(3.0f, 4.0f));
        const std::string text = ray.ToString();
        EXPECT_NE(text.find("Position"), std::string::npos);
        EXPECT_NE(text.find("Direction"), std::string::npos);
    }

    TEST(Ray2Tests, IntersectsBoundingRectangle_RayThroughCenter_ReturnsTrue)
    {
        const BoundingRectangle rect(Vector2(0.0f, 0.0f), Vector2(5.0f, 5.0f));
        const Ray2 ray(Vector2(-10.0f, 0.0f), Vector2(1.0f, 0.0f));

        float near = 0.0f;
        float far = 0.0f;
        const bool hit = ray.Intersects(rect, near, far);

        EXPECT_TRUE(hit);
        EXPECT_FLOAT_EQ(near, 5.0f);
        EXPECT_FLOAT_EQ(far, 15.0f);
    }

    TEST(Ray2Tests, IntersectsBoundingRectangle_RayMissesRectangle_ReturnsFalse)
    {
        const BoundingRectangle rect(Vector2(0.0f, 0.0f), Vector2(5.0f, 5.0f));
        const Ray2 ray(Vector2(-10.0f, 100.0f), Vector2(1.0f, 0.0f));

        float near = 0.0f;
        float far = 0.0f;
        const bool hit = ray.Intersects(rect, near, far);

        EXPECT_FALSE(hit);
        EXPECT_TRUE(std::isnan(near));
        EXPECT_TRUE(std::isnan(far));
    }

    TEST(Ray2Tests, IntersectsBoundingRectangle_OriginInsideRectangle_NearClampedToZero)
    {
        const BoundingRectangle rect(Vector2(0.0f, 0.0f), Vector2(5.0f, 5.0f));
        const Ray2 ray(Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f));

        float near = 0.0f;
        float far = 0.0f;
        const bool hit = ray.Intersects(rect, near, far);

        EXPECT_TRUE(hit);
        EXPECT_FLOAT_EQ(near, 0.0f);
        EXPECT_FLOAT_EQ(far, 5.0f);
    }
}
