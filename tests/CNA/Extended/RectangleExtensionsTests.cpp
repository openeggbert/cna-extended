// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/RectangleExtensionsTests.cs.
#include "CNA/Extended/RectangleExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(RectangleExtensionsTests, ClipReturnsIntersectionRectangle)
    {
        const Rectangle rect(0, 0, 10, 10);
        const Rectangle clip1(2, 2, 5, 5);
        const Rectangle clip2(2, 2, 15, 15);
        const Rectangle clip3(-2, -2, 5, 5);

        EXPECT_EQ(Clip(rect, clip1), Rectangle(2, 2, 5, 5));
        EXPECT_EQ(Clip(rect, clip2), Rectangle(2, 2, 8, 8));
        EXPECT_EQ(Clip(rect, clip3), Rectangle(0, 0, 3, 3));
    }

    TEST(RectangleExtensionsTests, NormalizeWithNegativeWidthAndHeightAdjustsPositionAndMakesDimensionsPositive)
    {
        const Rectangle rectangle(32, 32, -32, -32);
        const Rectangle actual = Normalize(rectangle);

        const Rectangle expected(0, 0, 32, 32);
        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleExtensionsTests, NormalizeWithNegativeWidthOnlyAdjustsXAndMakesWidthPositive)
    {
        const Rectangle rectangle(100, 50, -20, 30);
        const Rectangle actual = Normalize(rectangle);

        const Rectangle expected(80, 50, 20, 30);
        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleExtensionsTests, NormalizeWithNegativeHeightOnlyAdjustsYAndMakesHeightPositive)
    {
        const Rectangle rectangle(50, 100, 30, -20);
        const Rectangle actual = Normalize(rectangle);

        const Rectangle expected(50, 80, 30, 20);
        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleExtensionsTests, NormalizeWithPositiveDimensionsDoesNotModifyRectangle)
    {
        const Rectangle rectangle(10, 20, 30, 40);
        const Rectangle actual = Normalize(rectangle);

        const Rectangle expected(10, 20, 30, 40);
        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleExtensionsTests, NormalizeStaticMethodReturnsNormalizedRectangle)
    {
        const Rectangle rectangle(32, 32, -32, -32);

        const Rectangle actual = Normalize(rectangle);
        const Rectangle expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleExtensionsTests, NormalizeRefOutMethodOutputsNormalizedRectangle)
    {
        const Rectangle rectangle(32, 32, -32, -32);

        Rectangle actual;
        Normalize(rectangle, actual);
        const Rectangle expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }
}
