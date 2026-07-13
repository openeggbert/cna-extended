// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Fresh tests: upstream's Primitives/Size2Tests.cs is entirely commented out (0 active tests
// for this type family) -- see SizeTests.cpp's header comment for the same finding.
#include "CNA/Extended/SizeF.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(SizeFTests, ConstructorSetsWidthAndHeight)
    {
        const SizeF size(5.0f, 7.5f);
        EXPECT_FLOAT_EQ(size.Width, 5.0f);
        EXPECT_FLOAT_EQ(size.Height, 7.5f);
    }

    TEST(SizeFTests, EmptyIsZero)
    {
        EXPECT_TRUE(SizeF::Empty.getIsEmptyProperty());
    }

    TEST(SizeFTests, AdditionAndSubtraction)
    {
        const SizeF a(5.0f, 5.0f);
        const SizeF b(15.0f, 15.0f);
        EXPECT_EQ(a + b, SizeF(20.0f, 20.0f));
        EXPECT_EQ(b - a, SizeF(10.0f, 10.0f));
    }

    TEST(SizeFTests, VectorSubtraction)
    {
        const Vector2 point(10.0f, 10.0f);
        const SizeF size(3.0f, 4.0f);
        const Vector2 result = point - size;
        EXPECT_FLOAT_EQ(result.X, 7.0f);
        EXPECT_FLOAT_EQ(result.Y, 6.0f);
    }

    TEST(SizeFTests, MultiplyAndDivide)
    {
        const SizeF size(4.0f, 6.0f);
        EXPECT_EQ(size * 2.0f, SizeF(8.0f, 12.0f));
        EXPECT_EQ(size / 2.0f, SizeF(2.0f, 3.0f));
    }

    TEST(SizeFTests, Equality)
    {
        EXPECT_EQ(SizeF(1.0f, 2.0f), SizeF(1.0f, 2.0f));
        EXPECT_NE(SizeF(1.0f, 2.0f), SizeF(2.0f, 1.0f));
    }

    TEST(SizeFTests, GetHashCodeMatchesForEqualInstances)
    {
        EXPECT_EQ(SizeF(3.0f, 4.0f).GetHashCode(), SizeF(3.0f, 4.0f).GetHashCode());
    }

    TEST(SizeFTests, Vector2ConversionsRoundTrip)
    {
        const SizeF size(3.5f, 4.5f);
        const Vector2 vector = static_cast<Vector2>(size);
        EXPECT_FLOAT_EQ(vector.X, 3.5f);
        EXPECT_FLOAT_EQ(vector.Y, 4.5f);

        const SizeF fromVector(vector);
        EXPECT_EQ(fromVector, size);
    }

    TEST(SizeFTests, PointConversions)
    {
        const Point point(3, 4);
        const SizeF fromPoint(point);
        EXPECT_FLOAT_EQ(fromPoint.Width, 3.0f);
        EXPECT_FLOAT_EQ(fromPoint.Height, 4.0f);

        const Point backToPoint = static_cast<Point>(SizeF(3.9f, 4.9f));
        EXPECT_EQ(backToPoint.X, 3);
        EXPECT_EQ(backToPoint.Y, 4);
    }

    TEST(SizeFTests, ToStringFormat)
    {
        EXPECT_EQ(SizeF(5.0f, 7.0f).ToString(), "Width: 5.000000, Height: 7.000000");
    }
}
