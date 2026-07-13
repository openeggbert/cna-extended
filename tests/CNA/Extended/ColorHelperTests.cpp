// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ColorHelperTests.cs.
// xUnit's [Theory]/[InlineData] rows become GoogleTest TEST_P/INSTANTIATE_TEST_SUITE_P cases.
#include "CNA/Extended/ColorHelper.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace CNA::Extended
{
    namespace
    {
        struct HexColorCase
        {
            std::string hex;
            int r;
            int g;
            int b;
            int a;
        };
    }

    class FromHexLengthEightWithPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthEightWithPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthEightWithPrefixTests,
        ::testing::Values(HexColorCase{"#00000000", 0, 0, 0, 0}, HexColorCase{"#FFFFFFFF", 255, 255, 255, 255},
            HexColorCase{"#AABBCCFF", 170, 187, 204, 255}));

    class FromHexLengthSixWithPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthSixWithPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthSixWithPrefixTests,
        ::testing::Values(HexColorCase{"#000000", 0, 0, 0, 255}, HexColorCase{"#FFFFFF", 255, 255, 255, 255},
            HexColorCase{"#AABBCC", 170, 187, 204, 255}));

    class FromHexLengthThreeWithPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthThreeWithPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthThreeWithPrefixTests,
        ::testing::Values(HexColorCase{"#000", 0, 0, 0, 255}, HexColorCase{"#FFF", 255, 255, 255, 255},
            HexColorCase{"#ABC", 170, 187, 204, 255}));

    class FromHexLengthEightWithoutPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthEightWithoutPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthEightWithoutPrefixTests,
        ::testing::Values(HexColorCase{"00000000", 0, 0, 0, 0}, HexColorCase{"FFFFFFFF", 255, 255, 255, 255},
            HexColorCase{"AABBCCFF", 170, 187, 204, 255}));

    class FromHexLengthSixWithoutPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthSixWithoutPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthSixWithoutPrefixTests,
        ::testing::Values(HexColorCase{"000000", 0, 0, 0, 255}, HexColorCase{"FFFFFF", 255, 255, 255, 255},
            HexColorCase{"AABBCC", 170, 187, 204, 255}));

    class FromHexLengthThreeWithoutPrefixTests : public ::testing::TestWithParam<HexColorCase>
    {
    };

    TEST_P(FromHexLengthThreeWithoutPrefixTests, ReturnsCorrectColor)
    {
        const auto& c = GetParam();
        const Color expected(c.r, c.g, c.b, c.a);
        const Color actual = ColorHelper::FromHex(c.hex);
        EXPECT_EQ(expected, actual);
    }

    INSTANTIATE_TEST_SUITE_P(ColorHelperTests, FromHexLengthThreeWithoutPrefixTests,
        ::testing::Values(HexColorCase{"000", 0, 0, 0, 255}, HexColorCase{"FFF", 255, 255, 255, 255},
            HexColorCase{"ABC", 170, 187, 204, 255}));

    TEST(ColorHelperFromHexTests, EmptyReturnsTransparent)
    {
        const Color expected = Color::Transparent;
        const Color actual = ColorHelper::FromHex("");
        EXPECT_EQ(expected, actual);
    }

    TEST(ColorHelperFromHexTests, InvalidLengthThrowsArgumentException)
    {
        EXPECT_THROW((void)ColorHelper::FromHex("0"), std::invalid_argument);
        EXPECT_THROW((void)ColorHelper::FromHex("00"), std::invalid_argument);
        EXPECT_THROW((void)ColorHelper::FromHex("00000"), std::invalid_argument);
        EXPECT_THROW((void)ColorHelper::FromHex("0000000"), std::invalid_argument);
        EXPECT_THROW((void)ColorHelper::FromHex("000000000"), std::invalid_argument);
    }

    // Additional coverage beyond upstream: FromName and FromAbgr have no dedicated upstream test
    // class (only FromHex does), but they are real ported members -- cover them here.
    TEST(ColorHelperFromNameTests, KnownNameReturnsCorrectColorCaseInsensitively)
    {
        EXPECT_EQ(ColorHelper::FromName("Red"), Color::Red);
        EXPECT_EQ(ColorHelper::FromName("red"), Color::Red);
        EXPECT_EQ(ColorHelper::FromName("CORNFLOWERBLUE"), Color::CornflowerBlue);
    }

    TEST(ColorHelperFromNameTests, UnknownNameThrows)
    {
        EXPECT_THROW((void)ColorHelper::FromName("NotAColor"), std::invalid_argument);
    }

    TEST(ColorHelperFromAbgrTests, MatchesHtmlHexOrdering)
    {
        // Per ColorHelper::FromAbgr's doc comment: FromAbgr(0x123456FF) should read the same way
        // as HTML hex "#123456" (R=0x12, G=0x34, B=0x56), plus an appended alpha byte.
        const Color color = ColorHelper::FromAbgr(0x123456FFu);
        EXPECT_EQ(color.getRProperty(), 0x12);
        EXPECT_EQ(color.getGProperty(), 0x34);
        EXPECT_EQ(color.getBProperty(), 0x56);
        EXPECT_EQ(color.getAProperty(), 0xFF);
    }
}
