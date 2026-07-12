// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingBox2DTest.cs,
// covering the subset of upstream test cases that don't require Collision2D (Contains/
// Intersects/TryGetCollision), which is deferred -- see BoundingBox2D.hpp. Upstream has ~9
// additional Collision2D-dependent test methods not ported here; add them back alongside
// Collision2D in Phase 2.
#include "CNA/Extended/BoundingBox2D.hpp"

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

    TEST(BoundingBox2DTests, TranslateMovesBothCorners)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(10, 20));
        const BoundingBox2D translated = box.Translate(Vector2(5, -5));

        EXPECT_EQ(translated.Min, Vector2(5, -5));
        EXPECT_EQ(translated.Max, Vector2(15, 15));
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
