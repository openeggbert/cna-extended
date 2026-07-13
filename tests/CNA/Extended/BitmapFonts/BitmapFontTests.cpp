// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/BitmapFonts/BitmapFontTests.cs.
// Uses Texture2DRegion's test-only (string name, x, y, width, height) constructor -- upstream:
// `internal`, "used for unit tests only" -- matching upstream's own test exactly.
#include "CNA/Extended/BitmapFonts/BitmapFont.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::BitmapFonts
{
    namespace
    {
        BitmapFont CreateTestFont()
        {
            auto textureRegion = std::make_shared<Texture2DRegion>("Test Font", 219, 61, 16, 18);

            std::vector<std::shared_ptr<BitmapFontCharacter>> regions;
            // Extracted from the 'Impact' font. 'x' is particularly interesting because it has a
            // negative x offset.
            regions.push_back(std::make_shared<BitmapFontCharacter>(' ', textureRegion, 0, 0, 6));
            regions.push_back(std::make_shared<BitmapFontCharacter>('b', textureRegion, 0, 7, 17));
            regions.push_back(std::make_shared<BitmapFontCharacter>('f', textureRegion, 0, 7, 9));
            regions.push_back(std::make_shared<BitmapFontCharacter>('o', textureRegion, 0, 11, 16));
            regions.push_back(std::make_shared<BitmapFontCharacter>('x', textureRegion, -1, 11, 13));

            return BitmapFont("Impact", 32, 22, regions);
        }
    }

    TEST(BitmapFontTests, ConstructorSetsFaceAndLineHeight)
    {
        const BitmapFont font = CreateTestFont();

        EXPECT_EQ(font.getFaceProperty(), "Impact");
        EXPECT_EQ(font.getLineHeightProperty(), 22);
    }

    TEST(BitmapFontTests, MeasureStringSingleWord)
    {
        const BitmapFont font = CreateTestFont();
        const SizeF size = font.MeasureString("fox");

        EXPECT_FLOAT_EQ(size.Width, 40.0f);
        EXPECT_FLOAT_EQ(size.Height, static_cast<float>(font.getLineHeightProperty()));
    }

    TEST(BitmapFontTests, MeasureStringWithLetterSpacing)
    {
        BitmapFont font = CreateTestFont();
        font.setLetterSpacingProperty(3);

        const SizeF size = font.MeasureString("fox");

        EXPECT_FLOAT_EQ(size.Width, 46.0f);
        EXPECT_FLOAT_EQ(size.Height, static_cast<float>(font.getLineHeightProperty()));
    }

    TEST(BitmapFontTests, MeasureStringMultipleLines)
    {
        const BitmapFont font = CreateTestFont();
        const SizeF size = font.MeasureString("box fox\nbox of fox");

        EXPECT_FLOAT_EQ(size.Width, 123.0f);
        EXPECT_FLOAT_EQ(size.Height, static_cast<float>(font.getLineHeightProperty() * 2));
    }

    TEST(BitmapFontTests, MeasureStringEmptyString)
    {
        const BitmapFont font = CreateTestFont();
        const SizeF size = font.MeasureString(std::string());

        EXPECT_FLOAT_EQ(size.Width, 0.0f);
        EXPECT_FLOAT_EQ(size.Height, 0.0f);
    }

    // Test added for issue #695: https://github.com/craftworkgames/MonoGame.Extended/issues/695
    // Issue claims measure string does not account for space at the end of string.
    TEST(BitmapFontTests, MeasureStringSpaceAtEnd)
    {
        const BitmapFont font = CreateTestFont();

        const SizeF noSpaceAtEnd = font.MeasureString("bfox");
        const SizeF spaceAtEnd = font.MeasureString("bfox ");

        EXPECT_FALSE(noSpaceAtEnd.Equals(spaceAtEnd));
    }
}
