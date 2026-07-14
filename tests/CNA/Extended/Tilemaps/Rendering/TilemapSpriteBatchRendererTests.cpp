// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for TilemapSpriteBatchRenderer. These are
// fresh tests. Unlike most SpriteBatch-adjacent code elsewhere in this project (which has zero
// behavioral coverage because no headless GraphicsDevice/SpriteBatch test infra exists), a real
// `GraphicsDevice`/`SpriteBatch`/`Texture2D(GraphicsDevice&, w, h)` triple was confirmed this
// session to actually work end-to-end headlessly in this environment (real EasyGL-over-Mesa
// software rendering, no mocking needed) -- so these tests exercise real Begin/Draw/End calls,
// not just the SpriteBatch-independent surface (property round-trips, exceptions).
#include "CNA/Extended/Tilemaps/Rendering/TilemapSpriteBatchRenderer.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class TilemapSpriteBatchRendererTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
            }

            // Builds a 1-layer, 4x4 tile, 32x32-tile-size Tilemap using a single 2x2-tile (64x64px)
            // tileset texture, with a couple of tiles placed at (0,0) and (1,0).
            std::unique_ptr<Tilemap> BuildTilemap()
            {
                auto tilemap = std::make_unique<Tilemap>("TestMap", 4, 4, 32, 32, TilemapOrientation::Orthogonal);

                texture = std::make_unique<Texture2D>(graphicsDevice, 64, 64);
                auto tileset = std::make_unique<TilemapTileset>("TestSet", texture.get(), 32, 32, 4, 2);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto tileLayer = std::make_unique<TilemapTileLayer>("Tiles", 4, 4, 32, 32);
                tileLayer->SetTile(0, 0, TilemapTile(1));
                tileLayer->SetTile(1, 0, TilemapTile(2, TilemapTileFlipFlags::FlipHorizontally));
                tilemap->getLayersProperty().Add(std::move(tileLayer));

                return tilemap;
            }

            GraphicsDevice graphicsDevice;
            std::unique_ptr<Texture2D> texture;
        };
    }

    TEST_F(TilemapSpriteBatchRendererTest, DefaultPropertiesMatchUpstreamDefaults)
    {
        TilemapSpriteBatchRenderer renderer;

        EXPECT_EQ(renderer.getSamplerStateProperty(), &Microsoft::Xna::Framework::Graphics::SamplerState::PointClamp);
        EXPECT_EQ(renderer.getSpriteSortModeProperty(), Microsoft::Xna::Framework::Graphics::SpriteSortMode::Deferred);
        EXPECT_EQ(renderer.getEffectProperty(), nullptr);
    }

    TEST_F(TilemapSpriteBatchRendererTest, LoadTilemap_Null_ThrowsArgumentNullException)
    {
        TilemapSpriteBatchRenderer renderer;
        EXPECT_THROW(renderer.LoadTilemap(nullptr), System::ArgumentNullException);
    }

    TEST_F(TilemapSpriteBatchRendererTest, Draw_WithoutLoadedTilemap_ThrowsInvalidOperationException)
    {
        TilemapSpriteBatchRenderer renderer;
        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(spriteBatch, camera), System::InvalidOperationException);
    }

    TEST_F(TilemapSpriteBatchRendererTest, DrawLayerByName_UnknownName_ThrowsKeyNotFoundException)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.DrawLayer(spriteBatch, camera, std::string("DoesNotExist")), System::Collections::Generic::KeyNotFoundException);
    }

    TEST_F(TilemapSpriteBatchRendererTest, DrawLayerByIndex_OutOfRange_ThrowsArgumentOutOfRangeException)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.DrawLayer(spriteBatch, camera, 99), System::ArgumentOutOfRangeException);
        EXPECT_THROW(renderer.DrawLayer(spriteBatch, camera, -1), System::ArgumentOutOfRangeException);
    }

    TEST_F(TilemapSpriteBatchRendererTest, Draw_WithLoadedTileLayer_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(spriteBatch, camera));
    }

    TEST_F(TilemapSpriteBatchRendererTest, Draw_WithImageLayer_DoesNotThrow)
    {
        auto tilemap = std::make_unique<Tilemap>("ImgMap", 4, 4, 32, 32, TilemapOrientation::Orthogonal);
        texture = std::make_unique<Texture2D>(graphicsDevice, 16, 16);
        auto imageLayer = std::make_unique<TilemapImageLayer>("Background", texture.get(), Vector2::Zero);
        imageLayer->setRepeatXProperty(true);
        imageLayer->setRepeatYProperty(true);
        tilemap->getLayersProperty().Add(std::move(imageLayer));

        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.DrawLayer(spriteBatch, camera, std::string("Background")));
    }

    TEST_F(TilemapSpriteBatchRendererTest, Draw_WithObjectLayer_DoesNotThrow)
    {
        auto tilemap = BuildTilemap();
        auto objectLayer = std::make_unique<TilemapObjectLayer>("Objects");
        objectLayer->AddObject(std::make_unique<TilemapTileObject>(1, Vector2(10, 10), TilemapTile(1), Vector2(32, 32)));
        tilemap->getLayersProperty().Add(std::move(objectLayer));

        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.DrawLayer(spriteBatch, camera, std::string("Objects")));
    }

    TEST_F(TilemapSpriteBatchRendererTest, DrawLayers_WithMultipleNames_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        auto secondLayer = std::make_unique<TilemapTileLayer>("Tiles2", 4, 4, 32, 32);
        secondLayer->SetTile(0, 0, TilemapTile(1));
        tilemap->getLayersProperty().Add(std::move(secondLayer));

        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        const std::vector<std::string> names{"Tiles", "Tiles2"};
        EXPECT_NO_THROW(renderer.DrawLayers(spriteBatch, camera, names));
    }

    TEST_F(TilemapSpriteBatchRendererTest, UnloadTilemap_ThenDraw_ThrowsInvalidOperationException)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());
        renderer.UnloadTilemap();

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(spriteBatch, camera), System::InvalidOperationException);
    }

    TEST_F(TilemapSpriteBatchRendererTest, Update_WithNoAnimatedTiles_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> tilemap = BuildTilemap();
        TilemapSpriteBatchRenderer renderer;
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::GameTime gameTime;
        EXPECT_NO_THROW(renderer.Update(gameTime));
    }
}
