// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for SpriteBatch.Extensions. Fresh tests below. A plain
// `GraphicsDevice`/`SpriteBatch`/`Texture2D(GraphicsDevice&, w, h)` triple genuinely
// constructs and renders end-to-end headlessly in this environment (real EasyGL-over-Mesa
// software rendering, confirmed this session) -- so these tests exercise real
// Begin/Draw/End calls, not just compile-checks.
#include "CNA/Extended/Graphics/SpriteBatchExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class SpriteBatchExtensionsTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
                texture = std::make_unique<Texture2D>(graphicsDevice, 32, 32);
            }

            std::shared_ptr<Texture2DRegion> MakeRegion(int x, int y, int w, int h)
            {
                return std::make_shared<Texture2DRegion>(texture.get(), x, y, w, h);
            }

            GraphicsDevice graphicsDevice;
            std::unique_ptr<Texture2D> texture;
        };
    }

    TEST_F(SpriteBatchExtensionsTest, DrawNinePatch_DoesNotThrow)
    {
        std::vector<std::shared_ptr<Texture2DRegion>> patches(9);
        patches[NinePatch::TopLeft] = MakeRegion(0, 0, 8, 8);
        patches[NinePatch::TopMiddle] = MakeRegion(8, 0, 16, 8);
        patches[NinePatch::TopRight] = MakeRegion(24, 0, 8, 8);
        patches[NinePatch::MiddleLeft] = MakeRegion(0, 8, 8, 16);
        patches[NinePatch::Middle] = MakeRegion(8, 8, 16, 16);
        patches[NinePatch::MiddleRight] = MakeRegion(24, 8, 8, 16);
        patches[NinePatch::BottomLeft] = MakeRegion(0, 24, 8, 8);
        patches[NinePatch::BottomMiddle] = MakeRegion(8, 24, 16, 8);
        patches[NinePatch::BottomRight] = MakeRegion(24, 24, 8, 8);
        const NinePatch ninePatch(patches);

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, ninePatch, Rectangle(0, 0, 100, 60), Color::White));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawSprite_PositionRotationScaleOverload_DoesNotThrow)
    {
        const Sprite sprite(texture.get());

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(sprite, spriteBatch, Vector2(10, 10), 0.5f, Vector2(1.0f, 1.0f)));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawSprite_TransformOverload_DoesNotThrow)
    {
        const Sprite sprite(texture.get());
        Transform2 transform;
        transform.setPositionProperty(Vector2(5, 5));

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, sprite, transform));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawSprite_PositionRotationOverload_DoesNotThrow)
    {
        const Sprite sprite(texture.get());

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, sprite, Vector2(0, 0)));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawTextureWithSourceAndDestination_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(
            Draw(spriteBatch, *texture, Rectangle(0, 0, 16, 16), Rectangle(0, 0, 32, 32), Color::White, std::nullopt));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawTextureRegion_PositionOverload_DoesNotThrow)
    {
        const std::shared_ptr<Texture2DRegion> region = MakeRegion(0, 0, 16, 16);

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, region, Vector2(10, 10), Color::Red));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawTextureRegion_FullOverload_DoesNotThrow)
    {
        const std::shared_ptr<Texture2DRegion> region = MakeRegion(0, 0, 16, 16);

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, region, Vector2(10, 10), Color::Green, 0.3f, Vector2::Zero, Vector2(2.0f, 2.0f),
            SpriteEffects::None, 0.0f, std::nullopt));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawTextureRegion_DestinationRectangleOverload_DoesNotThrow)
    {
        const std::shared_ptr<Texture2DRegion> region = MakeRegion(0, 0, 16, 16);

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, region, Rectangle(0, 0, 64, 64), Color::Blue));
        spriteBatch.End();
    }

    TEST_F(SpriteBatchExtensionsTest, DrawTextureRegion_WithClippingRectangle_DoesNotThrow)
    {
        const std::shared_ptr<Texture2DRegion> region = MakeRegion(0, 0, 16, 16);

        SpriteBatch spriteBatch(graphicsDevice);
        spriteBatch.Begin();
        EXPECT_NO_THROW(Draw(spriteBatch, region, Vector2(10, 10), Color::White, Rectangle(0, 0, 20, 20)));
        spriteBatch.End();
    }
}
