// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for TilemapWorldSpriteBatchRenderer. These
// are fresh tests, using the same real headless GraphicsDevice/SpriteBatch/Texture2D pattern
// established in TilemapSpriteBatchRendererTests.cpp (see that file's header comment).
#include "CNA/Extended/Tilemaps/Rendering/TilemapWorldSpriteBatchRenderer.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
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
        class TilemapWorldSpriteBatchRendererTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
            }

            // Builds a single-room world: one 4x4, 32px-tile Tilemap positioned at @p worldPosition,
            // with one tile layer holding tiles at (0,0) and (1,0).
            std::unique_ptr<Tilemap> BuildRoom(const std::string& name, const Vector2& worldPosition)
            {
                auto tilemap = std::make_unique<Tilemap>(name, 4, 4, 32, 32, TilemapOrientation::Orthogonal);
                tilemap->setWorldPositionProperty(worldPosition);

                textures.push_back(std::make_unique<Texture2D>(graphicsDevice, 64, 64));
                auto tileset = std::make_unique<TilemapTileset>("TestSet", textures.back().get(), 32, 32, 4, 2);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto tileLayer = std::make_unique<TilemapTileLayer>("Tiles", 4, 4, 32, 32);
                tileLayer->SetTile(0, 0, TilemapTile(1));
                tileLayer->SetTile(1, 0, TilemapTile(2, TilemapTileFlipFlags::FlipDiagonally));
                tilemap->getLayersProperty().Add(std::move(tileLayer));

                return tilemap;
            }

            GraphicsDevice graphicsDevice;
            std::vector<std::unique_ptr<Texture2D>> textures;
        };
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, DefaultState_LastVisibleRoomCountIsZero)
    {
        TilemapWorldSpriteBatchRenderer renderer;
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 0);
        EXPECT_EQ(renderer.getSpriteSortModeProperty(), Microsoft::Xna::Framework::Graphics::SpriteSortMode::Deferred);
        EXPECT_EQ(renderer.getSamplerStateProperty(), &Microsoft::Xna::Framework::Graphics::SamplerState::PointClamp);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Draw_WithoutLoad_ThrowsInvalidOperationException)
    {
        TilemapWorldSpriteBatchRenderer renderer;
        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(spriteBatch, camera), System::InvalidOperationException);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Load_ThenDraw_DoesNotThrowAndCountsVisibleRoom)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        EXPECT_NO_THROW(renderer.Draw(spriteBatch, camera));
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 1);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Draw_RoomFarOutsideCamera_IsCulled)
    {
        std::unique_ptr<Tilemap> farRoom = BuildRoom("FarRoom", Vector2(100000.0f, 100000.0f));
        std::vector<Tilemap*> tilemaps{farRoom.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        renderer.Draw(spriteBatch, camera);
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 0);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Draw_MultipleRoomsSameDepth_BatchesInOneParallaxGroup)
    {
        std::unique_ptr<Tilemap> room1 = BuildRoom("Room1", Vector2::Zero);
        std::unique_ptr<Tilemap> room2 = BuildRoom("Room2", Vector2(128.0f, 0.0f));
        std::vector<Tilemap*> tilemaps{room1.get(), room2.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        renderer.Draw(spriteBatch, camera);
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 2);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Draw_WrongWorldDepth_DrawsNoRooms)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        room->setWorldDepthProperty(5);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        renderer.Draw(spriteBatch, camera, 0);
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 0);

        renderer.Draw(spriteBatch, camera, 5);
        EXPECT_EQ(renderer.getLastVisibleRoomCountProperty(), 1);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, LoadFromTilemapWorld_DoesNotThrow)
    {
        std::vector<std::unique_ptr<Tilemap>> levels;
        levels.push_back(BuildRoom("Room1", Vector2::Zero));
        TilemapWorld world(std::move(levels));

        TilemapWorldSpriteBatchRenderer renderer;
        EXPECT_NO_THROW(renderer.Load(world));

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        EXPECT_NO_THROW(renderer.Draw(spriteBatch, camera));
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Unload_ThenDraw_ThrowsInvalidOperationException)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);
        renderer.Unload();

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(spriteBatch, camera), System::InvalidOperationException);
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, Update_WithNoAnimatedTiles_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        Microsoft::Xna::Framework::GameTime gameTime;
        EXPECT_NO_THROW(renderer.Update(gameTime));
    }

    TEST_F(TilemapWorldSpriteBatchRendererTest, MoveConstruction_TransfersState)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldSpriteBatchRenderer renderer;
        renderer.Load(tilemaps);

        TilemapWorldSpriteBatchRenderer moved(std::move(renderer));

        Microsoft::Xna::Framework::Graphics::SpriteBatch spriteBatch(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);
        EXPECT_NO_THROW(moved.Draw(spriteBatch, camera));
        EXPECT_EQ(moved.getLastVisibleRoomCountProperty(), 1);
    }
}
