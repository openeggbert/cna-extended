// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test file exists for Thickness.cs. Fresh tests covering the ported API.
#include "CNA/Extended/Thickness.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(ThicknessTests, SingleValueConstructorSetsAllSides)
    {
        const Thickness t(5);
        EXPECT_EQ(t.getLeftProperty(), 5);
        EXPECT_EQ(t.getTopProperty(), 5);
        EXPECT_EQ(t.getRightProperty(), 5);
        EXPECT_EQ(t.getBottomProperty(), 5);
    }

    TEST(ThicknessTests, TwoValueConstructorSetsSymmetricSides)
    {
        const Thickness t(2, 3);
        EXPECT_EQ(t.getLeftProperty(), 2);
        EXPECT_EQ(t.getRightProperty(), 2);
        EXPECT_EQ(t.getTopProperty(), 3);
        EXPECT_EQ(t.getBottomProperty(), 3);
    }

    TEST(ThicknessTests, FourValueConstructorSetsEachSide)
    {
        const Thickness t(1, 2, 3, 4);
        EXPECT_EQ(t.getLeftProperty(), 1);
        EXPECT_EQ(t.getTopProperty(), 2);
        EXPECT_EQ(t.getRightProperty(), 3);
        EXPECT_EQ(t.getBottomProperty(), 4);
    }

    TEST(ThicknessTests, WidthAndHeightAndSize)
    {
        const Thickness t(1, 2, 3, 4);
        EXPECT_EQ(t.getWidthProperty(), 4);  // Left + Right = 1 + 3
        EXPECT_EQ(t.getHeightProperty(), 6); // Top + Bottom = 2 + 4
        const Size size = t.getSizeProperty();
        EXPECT_EQ(size.Width, 4);
        EXPECT_EQ(size.Height, 6);
    }

    TEST(ThicknessTests, ImplicitConversionFromInt)
    {
        const Thickness t = 7;
        EXPECT_EQ(t.getLeftProperty(), 7);
        EXPECT_EQ(t.getTopProperty(), 7);
        EXPECT_EQ(t.getRightProperty(), 7);
        EXPECT_EQ(t.getBottomProperty(), 7);
    }

    TEST(ThicknessTests, Equality)
    {
        EXPECT_EQ(Thickness(1, 2, 3, 4), Thickness(1, 2, 3, 4));
        EXPECT_NE(Thickness(1, 2, 3, 4), Thickness(4, 3, 2, 1));
    }

    TEST(ThicknessTests, GetHashCodeMatchesForEqualInstances)
    {
        EXPECT_EQ(Thickness(1, 2, 3, 4).GetHashCode(), Thickness(1, 2, 3, 4).GetHashCode());
    }

    TEST(ThicknessTests, FromValuesHandlesOneTwoAndFourElements)
    {
        EXPECT_EQ(Thickness::FromValues({5}), Thickness(5));
        EXPECT_EQ(Thickness::FromValues({2, 3}), Thickness(2, 3));
        EXPECT_EQ(Thickness::FromValues({1, 2, 3, 4}), Thickness(1, 2, 3, 4));
    }

    TEST(ThicknessTests, FromValuesThrowsOnInvalidCount)
    {
        EXPECT_THROW((void)Thickness::FromValues({1, 2, 3}), std::invalid_argument);
    }

    TEST(ThicknessTests, ParseHandlesCommaAndSpaceSeparators)
    {
        EXPECT_EQ(Thickness::Parse("5"), Thickness(5));
        EXPECT_EQ(Thickness::Parse("1, 2, 3, 4"), Thickness(1, 2, 3, 4));
        EXPECT_EQ(Thickness::Parse("1 2 3 4"), Thickness(1, 2, 3, 4));
    }

    TEST(ThicknessTests, DeconstructUsesTopRightBottomLeftOrder)
    {
        // Fidelity note: upstream's Deconstruct parameter order is (top, right, bottom, left),
        // not the constructor's (left, top, right, bottom) -- see Thickness.hpp's header comment.
        const Thickness t(1, 2, 3, 4); // left=1, top=2, right=3, bottom=4
        intcs top, right, bottom, left;
        t.Deconstruct(top, right, bottom, left);
        EXPECT_EQ(top, 2);
        EXPECT_EQ(right, 3);
        EXPECT_EQ(bottom, 4);
        EXPECT_EQ(left, 1);
    }

    TEST(ThicknessTests, ToStringUniformSingleValue)
    {
        EXPECT_EQ(Thickness(5).ToString(), "5");
    }

    TEST(ThicknessTests, ToStringSymmetricTwoValues)
    {
        EXPECT_EQ(Thickness(2, 3).ToString(), "2 3");
    }

    TEST(ThicknessTests, ToStringFourDistinctValues)
    {
        EXPECT_EQ(Thickness(1, 2, 3, 4).ToString(), "1, 3, 2, 4");
    }
}
