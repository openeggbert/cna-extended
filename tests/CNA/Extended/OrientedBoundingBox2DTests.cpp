// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/OrientedBoundingBox2DTest.cs,
// covering the subset of upstream test cases that don't require Collision2D, which is deferred
// -- see OrientedBoundingBox2D.hpp.
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"

#include <gtest/gtest.h>
#include <numbers>

namespace CNA::Extended
{
    TEST(OrientedBoundingBox2DTests, Constructor)
    {
        const OrientedBoundingBox2D obb(Vector2(1, 2), Vector2::UnitX, Vector2::UnitY, Vector2(3, 4));

        EXPECT_EQ(obb.Center, Vector2(1, 2));
        EXPECT_EQ(obb.AxisX, Vector2::UnitX);
        EXPECT_EQ(obb.AxisY, Vector2::UnitY);
        EXPECT_EQ(obb.HalfExtents, Vector2(3, 4));
    }

    TEST(OrientedBoundingBox2DTests, WidthAndHeight)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(3, 4));
        EXPECT_FLOAT_EQ(obb.getWidthProperty(), 6.0f);
        EXPECT_FLOAT_EQ(obb.getHeightProperty(), 8.0f);
    }

    TEST(OrientedBoundingBox2DTests, RotationOfAxisAlignedBoxIsZero)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        EXPECT_NEAR(obb.getRotationProperty(), 0.0f, 1e-5f);
    }

    TEST(OrientedBoundingBox2DTests, Area)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(3, 4));
        EXPECT_FLOAT_EQ(obb.getAreaProperty(), 48.0f);
    }

    TEST(OrientedBoundingBox2DTests, CreateFromRotation)
    {
        constexpr float halfPi = std::numbers::pi_v<float> / 2.0f;
        const OrientedBoundingBox2D obb = OrientedBoundingBox2D::CreateFromRotation(Vector2::Zero, halfPi, Vector2(1, 2));

        EXPECT_NEAR(obb.AxisX.X, 0.0f, 1e-5f);
        EXPECT_NEAR(obb.AxisX.Y, 1.0f, 1e-5f);
        EXPECT_NEAR(obb.getRotationProperty(), halfPi, 1e-5f);
    }

    TEST(OrientedBoundingBox2DTests, CreateFromBoundingBox2DIsAxisAligned)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(4, 4));
        const OrientedBoundingBox2D obb = OrientedBoundingBox2D::CreateFromBoundingBox2D(box);

        EXPECT_EQ(obb.Center, Vector2(2, 2));
        EXPECT_EQ(obb.AxisX, Vector2::UnitX);
        EXPECT_EQ(obb.AxisY, Vector2::UnitY);
        EXPECT_EQ(obb.HalfExtents, Vector2(2, 2));
    }

    TEST(OrientedBoundingBox2DTests, CreateMergedEnclosesBothCenters)
    {
        const OrientedBoundingBox2D a(Vector2(-2, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D b(Vector2(2, 0), Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D merged = OrientedBoundingBox2D::CreateMerged(a, b);

        // The merged box must be wide enough to span from a's left edge to b's right edge.
        EXPECT_GE(merged.getWidthProperty(), 6.0f - 1e-3f);
    }

    TEST(OrientedBoundingBox2DTests, GetCornersReturnsFourCorners)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const std::vector<Vector2> corners = obb.GetCorners();

        ASSERT_EQ(corners.size(), 4u);
        EXPECT_EQ(corners[0], Vector2(-1, -1));
        EXPECT_EQ(corners[1], Vector2(1, -1));
        EXPECT_EQ(corners[2], Vector2(1, 1));
        EXPECT_EQ(corners[3], Vector2(-1, 1));
    }

    TEST(OrientedBoundingBox2DTests, GetCornersOutParamMatchesReturnValue)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        Vector2 corners[4];
        obb.GetCorners(corners);

        const std::vector<Vector2> expected = obb.GetCorners();
        for (int i = 0; i < 4; i++)
        {
            EXPECT_EQ(corners[i], expected[static_cast<std::size_t>(i)]);
        }
    }

    TEST(OrientedBoundingBox2DTests, TranslateMovesCenterOnly)
    {
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D translated = obb.Translate(Vector2(3, 4));

        EXPECT_EQ(translated.Center, Vector2(3, 4));
        EXPECT_EQ(translated.AxisX, obb.AxisX);
        EXPECT_EQ(translated.HalfExtents, obb.HalfExtents);
    }

    TEST(OrientedBoundingBox2DTests, DeconstructReturnsComponents)
    {
        const OrientedBoundingBox2D obb(Vector2(1, 2), Vector2::UnitX, Vector2::UnitY, Vector2(3, 4));
        Vector2 center, axisX, axisY, halfExtents;
        obb.Deconstruct(center, axisX, axisY, halfExtents);

        EXPECT_EQ(center, Vector2(1, 2));
        EXPECT_EQ(axisX, Vector2::UnitX);
        EXPECT_EQ(axisY, Vector2::UnitY);
        EXPECT_EQ(halfExtents, Vector2(3, 4));
    }

    TEST(OrientedBoundingBox2DTests, EqualsAndOperators)
    {
        const OrientedBoundingBox2D a(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D b(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D c(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(2, 1));

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(OrientedBoundingBox2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const OrientedBoundingBox2D a(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));
        const OrientedBoundingBox2D b(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(1, 1));

        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }
}
