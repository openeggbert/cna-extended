// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for BitmapFont.Extensions. Fresh tests below. A plain
// `GraphicsDevice`/`SpriteBatch`/`Texture2D(GraphicsDevice&, w, h)` triple genuinely
// constructs and renders end-to-end headlessly in this environment (real EasyGL-over-Mesa
// software rendering, confirmed this session) -- so these tests exercise real
// Begin/Draw/End calls, not just compile-checks. Unlike `BitmapFontTests.cpp` (which uses
// `Texture2DRegion`'s test-only, texture-less constructor since it only checks metadata),
// these tests need a real `Texture2D`-backed region so `SpriteBatch::Draw` has something
// real to bind.
#include "CNA/Extended/BitmapFonts/BitmapFontExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/NotSupportedException.hpp"
#include "System/Text/StringBuilder.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::BitmapFonts
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class BitmapFontExtensionsTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
                texture = std::make_unique<Texture2D>(graphicsDevice, 64, 64);

                auto region = std::make_shared<Graphics::Texture2DRegion>(texture.get(), 0, 0, 16, 18);
                std::vector<std::shared_ptr<BitmapFontCharacter>> characters;
                characters.push_back(std::make_shared<BitmapFontCharacter>(' ', region, 0, 0, 6));
                characters.push_back(std::make_shared<BitmapFontCharacter>('A', region, 0, 0, 16));
                characters.push_back(std::make_shared<BitmapFontCharacter>('B', region, 0, 0, 16));

                font = std::make_unique<BitmapFont>("TestFont", 18, 22, characters);
            }

            GraphicsDevice graphicsDevice;
            std::unique_ptr<Texture2D> texture;
            std::unique_ptr<BitmapFont> font;
        };
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_FullControlStringOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, std::string("AB A"), Vector2(0, 0), Color::White, 0.0f, Vector2::Zero,
            Vector2(1.0f, 1.0f), SpriteEffects::None, 0.0f));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_FullControlStringBuilderOverload_DoesNotThrow)
    {
        System::Text::StringBuilder text;
        text.Append("BA");

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, text, Vector2(0, 0), Color::Red, 0.0f, Vector2::Zero,
            Vector2(1.0f, 1.0f), SpriteEffects::None, 0.0f));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_UniformScaleStringOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(
            DrawString(spriteBatch, *font, std::string("AB"), Vector2(0, 0), Color::Blue, 0.0f, Vector2::Zero, 2.0f, SpriteEffects::None, 0.0f));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_UniformScaleStringBuilderOverload_DoesNotThrow)
    {
        System::Text::StringBuilder text;
        text.Append("AB");

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(
            DrawString(spriteBatch, *font, text, Vector2(0, 0), Color::Green, 0.0f, Vector2::Zero, 1.5f, SpriteEffects::None, 0.0f));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_LayerDepthStringOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, std::string("AB"), Vector2(0, 0), Color::White, 0.0f));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_SimpleStringOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, std::string("AB"), Vector2(0, 0), Color::White));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_SimpleStringBuilderOverload_DoesNotThrow)
    {
        System::Text::StringBuilder text;
        text.Append("A");

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, text, Vector2(0, 0), Color::White));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_EmptyString_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawString(spriteBatch, *font, std::string(""), Vector2(0, 0), Color::White));
        spriteBatch.End();
    }

    TEST_F(BitmapFontExtensionsTest, DrawString_UnsupportedSpriteEffect_ThrowsNotSupportedException)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_THROW(DrawString(spriteBatch, *font, std::string("A"), Vector2(0, 0), Color::White, 0.0f, Vector2::Zero,
                         Vector2(1.0f, 1.0f), SpriteEffects::FlipHorizontally, 0.0f),
            System::NotSupportedException);
        spriteBatch.End();
    }
}
