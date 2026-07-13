// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended's tests/MonoGame.Extended.Tests/Primitives/Size2Tests.cs is entirely
// commented out upstream (0 active tests), matching the pattern already found for
// BoundingRectangleTests.cs/RectangleFTests.cs. Fresh tests covering what's ported here.
#include "CNA/Extended/Size.hpp"
#include "CNA/Extended/SizeF.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(SizeTests, ConstructorSetsWidthAndHeight)
    {
        const Size size(5, 7);
        EXPECT_EQ(size.Width, 5);
        EXPECT_EQ(size.Height, 7);
    }

    TEST(SizeTests, EmptyIsZero)
    {
        EXPECT_TRUE(Size::Empty.getIsEmptyProperty());
        EXPECT_EQ(Size::Empty.Width, 0);
        EXPECT_EQ(Size::Empty.Height, 0);
    }

    TEST(SizeTests, AdditionAndSubtraction)
    {
        const Size a(5, 5);
        const Size b(15, 15);
        EXPECT_EQ(a + b, Size(20, 20));
        EXPECT_EQ(b - a, Size(10, 10));
        EXPECT_EQ(Size::Add(a, b), Size(20, 20));
        EXPECT_EQ(Size::Subtract(b, a), Size(10, 10));
    }

    TEST(SizeTests, MultiplyAndDivide)
    {
        const Size size(4, 6);
        EXPECT_EQ(size * 2, Size(8, 12));
        EXPECT_EQ(size / 2, Size(2, 3));
    }

    TEST(SizeTests, Equality)
    {
        EXPECT_EQ(Size(1, 2), Size(1, 2));
        EXPECT_NE(Size(1, 2), Size(2, 1));
    }

    TEST(SizeTests, GetHashCodeMatchesForEqualInstances)
    {
        EXPECT_EQ(Size(3, 4).GetHashCode(), Size(3, 4).GetHashCode());
    }

    TEST(SizeTests, PointConversionsRoundTrip)
    {
        const Size size(3, 4);
        const Point point = static_cast<Point>(size);
        EXPECT_EQ(point.X, 3);
        EXPECT_EQ(point.Y, 4);

        const Size fromPoint(point);
        EXPECT_EQ(fromPoint, size);
    }

    TEST(SizeTests, ExplicitConversionFromSizeFTruncates)
    {
        const SizeF sizeF(3.9f, 4.1f);
        const Size size(sizeF);
        EXPECT_EQ(size.Width, 3);
        EXPECT_EQ(size.Height, 4);
    }

    TEST(SizeTests, ToStringFormat)
    {
        EXPECT_EQ(Size(5, 7).ToString(), "Width: 5, Height: 7");
    }
}
