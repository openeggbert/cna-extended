// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Text3DEXT/BuildText3DMeshEXT/TextBillboardRenderSystemEXT
// (see 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
// Builds a minimal BitmapFont by hand via its public characters-list constructor (no .fnt
// file needed), matching this project's established "hand-build minimal test fixtures for
// GPU-backed types" idiom (see RenderSystem3DEXTTests.cpp's TestTriangleModel,
// AnimationSystem3DEXTTests.cpp's MakeMovingTriangleModel).
#include "CNA/Extended/World3DEXT/Text3DEXT.hpp"

#include "CNA/Extended/BitmapFonts/BitmapFont.hpp"
#include "CNA/Extended/BitmapFonts/BitmapFontCharacter.hpp"
#include "CNA/Extended/ECS/World.hpp"
#include "CNA/Extended/ECS/WorldBuilder.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TextBillboardRenderSystemEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using BitmapFonts::BitmapFont;
    using BitmapFonts::BitmapFontCharacter;
    using CNA::Extended::Graphics::Texture2DRegion;
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using ECS::World;
    using ECS::WorldBuilder;
    using System::TimeSpan;

    namespace
    {
        // A 2-character "AB" font: 'A' -> left half of a 4x2 texture (solid red), 'B' ->
        // right half (solid green), each glyph 2x2 pixels wide/tall with a 2px advance.
        BitmapFont BuildTestFont(Texture2D& texture)
        {
            auto regionA = std::make_shared<Texture2DRegion>(&texture, Rectangle(0, 0, 2, 2));
            auto regionB = std::make_shared<Texture2DRegion>(&texture, Rectangle(2, 0, 2, 2));

            const std::vector<std::shared_ptr<BitmapFontCharacter>> characters = {
                std::make_shared<BitmapFontCharacter>('A', regionA, 0, 0, 2),
                std::make_shared<BitmapFontCharacter>('B', regionB, 0, 0, 2),
            };

            return BitmapFont("test-font", 2, 2, characters);
        }

        Texture2D BuildTwoGlyphTexture(GraphicsDevice& graphicsDevice)
        {
            Texture2D texture(graphicsDevice, 4, 2);
            const std::vector<Color> pixels = {
                Color::Red, Color::Red, Color::Green, Color::Green,
                Color::Red, Color::Red, Color::Green, Color::Green,
            };
            texture.SetData(pixels.data(), static_cast<int>(pixels.size()));
            return texture;
        }
    }

    TEST(Text3DEXTMeshTests, BuildText3DMeshEXT_EmptyString_ProducesEmptyMesh)
    {
        GraphicsDevice graphicsDevice;
        Texture2D texture = BuildTwoGlyphTexture(graphicsDevice);
        const BitmapFont font = BuildTestFont(texture);

        const Text3DMeshEXT mesh = BuildText3DMeshEXT(graphicsDevice, font, "");

        EXPECT_EQ(mesh.PrimitiveCount, 0);
        EXPECT_EQ(mesh.VertexBuffer, nullptr);
        EXPECT_EQ(mesh.IndexBuffer, nullptr);
    }

    TEST(Text3DEXTMeshTests, BuildText3DMeshEXT_TwoCharacterString_ProducesTwoGlyphQuads)
    {
        GraphicsDevice graphicsDevice;
        Texture2D texture = BuildTwoGlyphTexture(graphicsDevice);
        const BitmapFont font = BuildTestFont(texture);

        const Text3DMeshEXT mesh = BuildText3DMeshEXT(graphicsDevice, font, "AB");

        EXPECT_EQ(mesh.PrimitiveCount, 4); // 2 triangles per glyph x 2 glyphs
        ASSERT_NE(mesh.VertexBuffer, nullptr);
        ASSERT_NE(mesh.IndexBuffer, nullptr);
        EXPECT_EQ(mesh.VertexBuffer->getVertexCountProperty(), 8); // 4 vertices per glyph x 2 glyphs
        EXPECT_EQ(mesh.Texture, &texture);
    }

    namespace
    {
        class TextBillboardRenderSystemEXTTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
                graphicsDevice.setRasterizerStateProperty(RasterizerState::CullNone);

                camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
                camera.setTargetProperty(Vector3::Zero);
                camera.setFieldOfViewProperty(MathHelper::PiOver4);
                camera.setAspectRatioProperty(800.0f / 480.0f);
                camera.setNearPlaneProperty(0.1f);
                camera.setFarPlaneProperty(100.0f);
            }

            std::pair<std::vector<Color>, int> RenderToPixels(World& world, const GameTime& gameTime)
            {
                const int width = graphicsDevice.getViewportProperty().getWidthProperty();
                const int height = graphicsDevice.getViewportProperty().getHeightProperty();

                RenderTarget2D rt(graphicsDevice, width, height);
                graphicsDevice.SetRenderTarget(&rt);
                graphicsDevice.Clear(Color::Black);

                world.Draw(gameTime);

                std::vector<Color> pixels(
                    static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Color::Transparent);
                graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));

                graphicsDevice.SetRenderTarget(nullptr);

                return {std::move(pixels), width};
            }

            GraphicsDevice graphicsDevice;
            Camera3DEXT camera;
        };

        bool AnyPixelMatches(const std::vector<Color>& pixels, const Color& color)
        {
            return std::any_of(pixels.begin(), pixels.end(), [&](const Color& p) { return p == color; });
        }
    }

    TEST_F(TextBillboardRenderSystemEXTTest, VisibleText_IsDrawnWithGlyphColors)
    {
        Texture2D texture = BuildTwoGlyphTexture(graphicsDevice);
        const BitmapFont font = BuildTestFont(texture);
        Text3DMeshEXT mesh = BuildText3DMeshEXT(graphicsDevice, font, "AB");
        ASSERT_NE(mesh.VertexBuffer, nullptr);

        WorldBuilder builder;
        builder.AddSystem(std::make_unique<TextBillboardRenderSystemEXT>(graphicsDevice, camera));
        const std::unique_ptr<World> world = builder.Build();
        world->Initialize();

        ECS::Entity& entity = world->CreateEntity();
        Text3DEXT textComponent;
        textComponent.VertexBufferEXT = mesh.VertexBuffer.get();
        textComponent.IndexBufferEXT = mesh.IndexBuffer.get();
        textComponent.PrimitiveCountEXT = mesh.PrimitiveCount;
        textComponent.TextureEXT = mesh.Texture;
        textComponent.ScaleEXT = 2.0f;
        entity.Attach(&textComponent);

        GameTime gameTime(TimeSpan::Zero, TimeSpan::FromMilliseconds(16));
        world->Update(gameTime);

        const auto [pixels, width] = RenderToPixels(*world, gameTime);
        (void)width;

        EXPECT_TRUE(AnyPixelMatches(pixels, Color::Red));
        EXPECT_TRUE(AnyPixelMatches(pixels, Color::Green));
    }
}
