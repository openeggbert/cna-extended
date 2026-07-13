// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ColorExtensionsTests.cs.
#include "CNA/Extended/ColorExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(ColorExtensionsToHexTests, ColorTransparentReturnsCorrectHex)
    {
        const Color transparent(0, 0, 0, 0);
        EXPECT_EQ(ToHex(transparent), "#00000000");
    }

    TEST(ColorExtensionsToHexTests, ColorWhiteReturnsCorrectHex)
    {
        const Color white(255, 255, 255);
        EXPECT_EQ(ToHex(white), "#ffffffff");
    }

    TEST(ColorExtensionsToHexTests, ColorBlackReturnsCorrectHex)
    {
        const Color black(0, 0, 0);
        EXPECT_EQ(ToHex(black), "#000000ff");
    }

    TEST(ColorExtensionsToHexTests, ColorRedReturnsCorrectHex)
    {
        const Color red(255, 0, 0);
        EXPECT_EQ(ToHex(red), "#ff0000ff");
    }

    TEST(ColorExtensionsToHexTests, ColorGreenReturnsCorrectHex)
    {
        const Color green(0, 255, 0);
        EXPECT_EQ(ToHex(green), "#00ff00ff");
    }

    TEST(ColorExtensionsToHexTests, ColorBlueReturnsCorrectHex)
    {
        const Color blue(0, 0, 255);
        EXPECT_EQ(ToHex(blue), "#0000ffff");
    }

    TEST(ColorExtensionsToHexTests, ColorReturnsCorrectHex)
    {
        const Color color(170, 187, 204, 128);
        EXPECT_EQ(ToHex(color), "#aabbcc80");
    }
}
