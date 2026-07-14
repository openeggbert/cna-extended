// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for TilemapRenderer3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Real headless render tests, matching
// TilemapIntegrationTests.cpp's/CubeMeshRenderSystemEXTTests.cpp's established idiom.
#include "CNA/Extended/World3DEXT/TilemapRenderer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CubeMeshRenderSystemEXT.hpp"
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

#include <algorithm>
#include <functional>
#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class TilemapRenderer3DEXTTest : public ::testing::Test
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

            std::pair<std::vector<Color>, int> RenderToPixels(const std::function<void()>& render)
            {
                const int width = graphicsDevice.getViewportProperty().getWidthProperty();
                const int height = graphicsDevice.getViewportProperty().getHeightProperty();

                RenderTarget2D rt(graphicsDevice, width, height);
                graphicsDevice.SetRenderTarget(&rt);
                graphicsDevice.Clear(Color::Black);

                render();

                std::vector<Color> pixels(
                    static_cast<std::size_t>(width) * static_cast<std::size_t>(height), Color::Transparent);
                graphicsDevice.GetBackBufferData(pixels.data(), static_cast<int>(pixels.size()));

                graphicsDevice.SetRenderTarget(nullptr);

                return {std::move(pixels), width};
            }

            GraphicsDevice graphicsDevice;
            Camera3DEXT camera;
        };

        Color GetPixel(const std::vector<Color>& pixels, int width, int x, int y)
        {
            return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
        }
    }

    TEST_F(TilemapRenderer3DEXTTest, VisibleTile_IsDrawnAtItsWorldPosition)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        Tilemap3DEXT tilemap(Vector3(4.0f, 4.0f, 4.0f));
        tilemap.SetTileEXT(0, 0, 0, 1);

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        CubeMeshRenderSystemEXT cubeRenderSystem(graphicsDevice, camera);
        TilemapRenderer3DEXT renderer(cubeRenderSystem);

        const auto [pixels, width] = RenderToPixels([&]() { renderer.Draw(camera, tilemap, tileset); });
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center = GetPixel(pixels, width, width / 2, height / 2);

        EXPECT_GT(center.getRProperty(), 0);
        EXPECT_GT(center.getGProperty(), 0);
        EXPECT_GT(center.getBProperty(), 0);
    }

    TEST_F(TilemapRenderer3DEXTTest, TileOutsideFrustum_IsCulledAndNotDrawn)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        Tilemap3DEXT tilemap(Vector3(1.0f, 1.0f, 1.0f));
        tilemap.SetTileEXT(10000, 0, 0, 1); // far outside the camera's frustum

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        CubeMeshRenderSystemEXT cubeRenderSystem(graphicsDevice, camera);
        TilemapRenderer3DEXT renderer(cubeRenderSystem);

        const auto [pixels, width] = RenderToPixels([&]() { renderer.Draw(camera, tilemap, tileset); });
        (void)width;

        EXPECT_TRUE(std::none_of(pixels.begin(), pixels.end(),
                                  [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; }));
    }

    TEST_F(TilemapRenderer3DEXTTest, TileWithNoTextureAssigned_DoesNotThrow)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 99); // tile ID with no registered texture

        TilemapTileset3DEXT tileset;
        CubeMeshRenderSystemEXT cubeRenderSystem(graphicsDevice, camera);
        TilemapRenderer3DEXT renderer(cubeRenderSystem);

        EXPECT_NO_THROW(RenderToPixels([&]() { renderer.Draw(camera, tilemap, tileset); }));
    }
}
