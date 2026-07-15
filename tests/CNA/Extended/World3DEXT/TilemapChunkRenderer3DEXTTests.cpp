// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for TilemapChunkRenderer3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here. Real headless render tests, matching
// TilemapRenderer3DEXTTests.cpp's established idiom.
#include "CNA/Extended/World3DEXT/TilemapChunkRenderer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"
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
        class TilemapChunkRenderer3DEXTTest : public ::testing::Test
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

    TEST_F(TilemapChunkRenderer3DEXTTest, VisibleTile_IsDrawnAtItsWorldPosition)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        Tilemap3DEXT tilemap(Vector3(4.0f, 4.0f, 4.0f));
        tilemap.SetTileEXT(0, 0, 0, 1);

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        const auto [pixels, width] = RenderToPixels([&]() { renderer.Draw(); });
        const int height = graphicsDevice.getViewportProperty().getHeightProperty();
        const Color center = GetPixel(pixels, width, width / 2, height / 2);

        EXPECT_GT(center.getRProperty(), 0);
        EXPECT_GT(center.getGProperty(), 0);
        EXPECT_GT(center.getBProperty(), 0);
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, ChunkOutsideFrustum_IsCulledAndNotDrawn)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        Tilemap3DEXT tilemap(Vector3(1.0f, 1.0f, 1.0f));
        tilemap.SetTileEXT(10000, 0, 0, 1); // far outside the camera's frustum

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        const auto [pixels, width] = RenderToPixels([&]() { renderer.Draw(); });
        (void)width;

        EXPECT_TRUE(std::none_of(pixels.begin(), pixels.end(),
                                  [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; }));
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, TileWithNoTextureAssigned_DoesNotThrow)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 99); // tile ID with no registered texture

        TilemapTileset3DEXT tileset;
        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_NO_THROW(RenderToPixels([&]() { renderer.Draw(); }));
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, SingleTile_ProducesOneBatch)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_EQ(renderer.GetChunkBatchCountEXT(), 1);
        EXPECT_EQ(renderer.GetTotalPrimitiveCountEXT(), 12); // 6 faces x 2 triangles, nothing hidden
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, TwoTilesInDifferentChunks_ProducesTwoBatches)
    {
        // Default chunk size is 16 -- tile 0 and tile 100 land in different chunks.
        Texture2D whiteTex(graphicsDevice, 1, 1);
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(100, 0, 0, 1);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_EQ(renderer.GetChunkBatchCountEXT(), 2);
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, TwoAdjacentTilesSameChunkSameTexture_ProducesOneBatch)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(1, 0, 0, 1);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_EQ(renderer.GetChunkBatchCountEXT(), 1);
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, SameChunkDifferentTextures_ProducesTwoBatches)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        Texture2D blackTex(graphicsDevice, 1, 1);
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(1, 0, 0, 2);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);
        tileset.SetTileTextureEXT(2, &blackTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_EQ(renderer.GetChunkBatchCountEXT(), 2);
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, AdjacentTiles_HideSharedInternalFaces)
    {
        // 3 tiles in a row: 2 shared boundaries, each hiding one face from both neighbors
        // (2 boundaries x 2 hidden faces x 2 triangles/face = 8 fewer triangles than 3
        // fully-isolated tiles would produce).
        Texture2D whiteTex(graphicsDevice, 1, 1);
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(1, 0, 0, 1);
        tilemap.SetTileEXT(2, 0, 0, 1);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);

        EXPECT_EQ(renderer.GetTotalPrimitiveCountEXT(), 3 * 12 - 8);
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, RebuildEXT_CalledTwice_ReplacesOldBatches)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        renderer.RebuildEXT(tilemap, tileset);
        EXPECT_EQ(renderer.GetTotalPrimitiveCountEXT(), 12);

        tilemap.SetTileEXT(1, 0, 0, 1);
        renderer.RebuildEXT(tilemap, tileset);
        EXPECT_EQ(renderer.GetTotalPrimitiveCountEXT(), 2 * 12 - 4); // one shared boundary now
    }

    TEST_F(TilemapChunkRenderer3DEXTTest, NegativeTileCoordinates_DoesNotThrowAndIsDrawn)
    {
        Texture2D whiteTex(graphicsDevice, 1, 1);
        const std::vector<Color> pixelData(1, Color::White);
        whiteTex.SetData(pixelData.data(), 1);

        Tilemap3DEXT tilemap(Vector3(4.0f, 4.0f, 4.0f));
        tilemap.SetTileEXT(-1, -1, -1, 1);
        tilemap.SetTileEXT(-20, -20, -20, 1); // a different (negative) chunk

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(1, &whiteTex);

        TilemapChunkRenderer3DEXT renderer(graphicsDevice, camera);
        EXPECT_NO_THROW(renderer.RebuildEXT(tilemap, tileset));

        EXPECT_EQ(renderer.GetChunkBatchCountEXT(), 2);

        // Both tiles sit off-center (world positions (-4,-4,-4) and (-80,-80,-80)) rather
        // than at the origin, so just confirm something was drawn rather than checking the
        // exact center pixel.
        const auto [pixels, width] = RenderToPixels([&]() { renderer.Draw(); });
        (void)width;
        EXPECT_TRUE(std::any_of(pixels.begin(), pixels.end(),
                                 [](const Color& p) { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; }));
    }
}
