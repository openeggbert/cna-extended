// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/HslColorTests.cs. Upstream's
// "AreEqualObjectMethod.WhenGivenObjectOfAntotherType_ReturnsFalse" test (comparing against a
// boxed DateTime via the object-typed Equals override) has no C++ equivalent -- matches the
// precedent elsewhere in this project that C#'s `object obj` Equals overload is not ported.
#include "CNA/Extended/HslColor.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(HslColorConstructorTests, WhenGivenValuesReturnsInitializedColour)
    {
        const HslColor colour(1.0f, 1.0f, 1.0f);
        EXPECT_FLOAT_EQ(colour.getHProperty(), 1.0f);
        EXPECT_FLOAT_EQ(colour.getSProperty(), 1.0f);
        EXPECT_FLOAT_EQ(colour.getLProperty(), 1.0f);
    }

    TEST(HslColorAreEqualColourMethodTests, WhenGivenEqualValuesReturnsTrue)
    {
        const HslColor x(360.0f, 1.0f, 1.0f);
        const HslColor y(360.0f, 1.0f, 1.0f);
        EXPECT_EQ(x, y);
    }

    TEST(HslColorAreEqualColourMethodTests, WhenGivenDifferentValuesReturnsFalse)
    {
        const HslColor x(0.0f, 1.0f, 0.0f);
        const HslColor y(360.0f, 1.0f, 1.0f);
        EXPECT_FALSE(x.Equals(y));
    }

    TEST(HslColorGetHashCodeMethodTests, WhenObjectsAreDifferentYieldsDifferentHashCodes)
    {
        const HslColor x(0.0f, 1.0f, 0.0f);
        const HslColor y(360.0f, 1.0f, 1.0f);
        EXPECT_NE(x.GetHashCode(), y.GetHashCode());
    }

    TEST(HslColorGetHashCodeMethodTests, WhenObjectsAreSameYieldsIdenticalHashCodes)
    {
        const HslColor x(180.0f, 0.5f, 0.5f);
        const HslColor y(180.0f, 0.5f, 0.5f);
        EXPECT_EQ(x.GetHashCode(), y.GetHashCode());
    }

    struct HslColorToStringCase
    {
        float h;
        float s;
        float l;
        std::string expected;
    };

    class HslColorToStringMethodTests : public ::testing::TestWithParam<HslColorToStringCase>
    {
    };

    TEST_P(HslColorToStringMethodTests, ReturnsCorrectValue)
    {
        const auto& c = GetParam();
        const HslColor colour(c.h, c.s, c.l);
        EXPECT_EQ(colour.ToString(), c.expected);
    }

    INSTANTIATE_TEST_SUITE_P(HslColorTests, HslColorToStringMethodTests,
        ::testing::Values(HslColorToStringCase{360.0f, 1.0f, 1.0f, "H:360.0\xC2\xB0 S:100.0 L:100.0"},
            HslColorToStringCase{180.0f, 0.5f, 0.5f, "H:180.0\xC2\xB0 S:50.0 L:50.0"},
            HslColorToStringCase{0.0f, 0.0f, 0.0f, "H:0.0\xC2\xB0 S:0.0 L:0.0"}));

    struct HslColorToRgbCase
    {
        float h;
        float s;
        float l;
        std::string expected;
    };

    class HslColorToRgbMethodTests : public ::testing::TestWithParam<HslColorToRgbCase>
    {
    };

    TEST_P(HslColorToRgbMethodTests, ReturnsCorrectValue)
    {
        const auto& c = GetParam();
        const HslColor hslColour(c.h, c.s, c.l);
        const Color rgbColor = HslColor::ToRgb(hslColour);
        EXPECT_EQ(rgbColor.ToString(), c.expected);
    }

    INSTANTIATE_TEST_SUITE_P(HslColorTests, HslColorToRgbMethodTests,
        ::testing::Values(HslColorToRgbCase{0.0f, 1.0f, 0.5f, "{R:255 G:0 B:0 A:255}"},   // Color.Red
            HslColorToRgbCase{360.0f, 1.0f, 0.5f, "{R:255 G:0 B:0 A:255}"},               // Color.Red
            HslColorToRgbCase{120.0f, 1.0f, 0.5f, "{R:0 G:255 B:0 A:255}"}));             // Color.Lime

    TEST(HslColorToRgbMethodTests, FromRgbAndToRgbWorksCorrectly)
    {
        const HslColor blueHsl = HslColor::FromRgb(Color::Blue);
        const Color blueRgb = HslColor::ToRgb(blueHsl);
        EXPECT_EQ(blueRgb, Color::Blue);
    }
}
