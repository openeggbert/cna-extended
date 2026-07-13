// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for PrimitivesHelper.cs (it's `internal`).
// These tests cover each method directly.
#include "CNA/Extended/PrimitivesHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(PrimitivesHelperTests, IntersectsSlabDetectsOverlap)
    {
        float rayMin = 0.0f;
        float rayMax = 100.0f;
        const bool result = PrimitivesHelper::IntersectsSlab(0.0f, 1.0f, 5.0f, 10.0f, rayMin, rayMax);
        EXPECT_TRUE(result);
        EXPECT_FLOAT_EQ(rayMin, 5.0f);
        EXPECT_FLOAT_EQ(rayMax, 10.0f);
    }

    TEST(PrimitivesHelperTests, IntersectsSlabDetectsNoOverlap)
    {
        float rayMin = 20.0f;
        float rayMax = 25.0f;
        const bool result = PrimitivesHelper::IntersectsSlab(0.0f, 1.0f, 5.0f, 10.0f, rayMin, rayMax);
        EXPECT_FALSE(result);
    }

    TEST(PrimitivesHelperTests, CreateRectangleFromPointsComputesMinMax)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(1, 8), Vector2(6, 2)};
        Vector2 minimum;
        Vector2 maximum;
        PrimitivesHelper::CreateRectangleFromPoints(points, minimum, maximum);
        EXPECT_EQ(minimum, Vector2(1, 2));
        EXPECT_EQ(maximum, Vector2(6, 8));
    }

    TEST(PrimitivesHelperTests, CreateRectangleFromEmptyPointsReturnsZero)
    {
        Vector2 minimum(1, 1);
        Vector2 maximum(1, 1);
        PrimitivesHelper::CreateRectangleFromPoints({}, minimum, maximum);
        EXPECT_EQ(minimum, Vector2::Zero);
        EXPECT_EQ(maximum, Vector2::Zero);
    }

    TEST(PrimitivesHelperTests, TransformRectangleTranslates)
    {
        Vector2 center(0, 0);
        Vector2 halfExtents(5, 5);
        const Matrix3x2 translation = Matrix3x2::CreateTranslation(Vector2(10, 20));
        PrimitivesHelper::TransformRectangle(center, halfExtents, translation);
        EXPECT_EQ(center, Vector2(10, 20));
        EXPECT_EQ(halfExtents, Vector2(5, 5));
    }

    TEST(PrimitivesHelperTests, TransformOrientedRectangleResetsTranslationOnOrientation)
    {
        Vector2 center(0, 0);
        Matrix3x2 orientation = Matrix3x2::Identity;
        const Matrix3x2 transform = Matrix3x2::CreateTranslation(Vector2(10, 20));
        PrimitivesHelper::TransformOrientedRectangle(center, orientation, transform);
        EXPECT_EQ(center, Vector2(10, 20));
        EXPECT_FLOAT_EQ(orientation.M31, 0.0f);
        EXPECT_FLOAT_EQ(orientation.M32, 0.0f);
    }

    TEST(PrimitivesHelperTests, SquaredDistanceToPointFromRectangleIsZeroWhenInside)
    {
        const float distance = PrimitivesHelper::SquaredDistanceToPointFromRectangle(Vector2(0, 0), Vector2(10, 10), Vector2(5, 5));
        EXPECT_FLOAT_EQ(distance, 0.0f);
    }

    TEST(PrimitivesHelperTests, SquaredDistanceToPointFromRectangleIsPositiveWhenOutside)
    {
        const float distance = PrimitivesHelper::SquaredDistanceToPointFromRectangle(Vector2(0, 0), Vector2(10, 10), Vector2(13, 0));
        EXPECT_FLOAT_EQ(distance, 9.0f);
    }

    TEST(PrimitivesHelperTests, ClosestPointToPointFromRectangleClampsToBox)
    {
        Vector2 result;
        PrimitivesHelper::ClosestPointToPointFromRectangle(Vector2(0, 0), Vector2(10, 10), Vector2(-5, 20), result);
        EXPECT_EQ(result, Vector2(0, 10));
    }

    TEST(PrimitivesHelperTests, ClosestPointToPointFromRectangleKeepsInteriorPoint)
    {
        Vector2 result;
        PrimitivesHelper::ClosestPointToPointFromRectangle(Vector2(0, 0), Vector2(10, 10), Vector2(3, 4), result);
        EXPECT_EQ(result, Vector2(3, 4));
    }
}
