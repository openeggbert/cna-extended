// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for TilemapWorldRenderer. These are fresh
// tests, using the same real headless GraphicsDevice/Texture2D pattern established in
// TilemapWorldSpriteBatchRendererTests.cpp (see that file's header comment) and
// TilemapRendererTests.cpp (see that file's header comment for the DefaultEffect -> BasicEffect
// substitution these draw calls exercise).
#include "CNA/Extended/Tilemaps/Rendering/TilemapWorldRenderer.hpp"

#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Graphics::BasicEffect;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class TilemapWorldRendererTest : public ::testing::Test
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

    TEST_F(TilemapWorldRendererTest, DefaultProperties_MatchUpstreamDefaults)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        EXPECT_EQ(renderer.getSamplerStateProperty(), &SamplerState::PointClamp);
        EXPECT_EQ(renderer.getBlendStateProperty().getColorSourceBlendProperty(),
                  BlendState::NonPremultiplied.getColorSourceBlendProperty());
    }

    TEST_F(TilemapWorldRendererTest, Draw_WithoutLoad_ThrowsInvalidOperationException)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(camera), System::InvalidOperationException);
    }

    TEST_F(TilemapWorldRendererTest, Load_ThenDraw_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(tilemaps);

        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapWorldRendererTest, Draw_MultipleRoomsSameDepth_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> room1 = BuildRoom("Room1", Vector2::Zero);
        std::unique_ptr<Tilemap> room2 = BuildRoom("Room2", Vector2(128.0f, 0.0f));
        std::vector<Tilemap*> tilemaps{room1.get(), room2.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(tilemaps);

        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapWorldRendererTest, Draw_WrongWorldDepth_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        room->setWorldDepthProperty(5);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(tilemaps);

        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2::Zero);

        // Depth 0 (the default) has no matching batches -- must draw nothing without throwing.
        EXPECT_NO_THROW(renderer.Draw(camera, 0));
        // Depth 5 has the room's batches.
        EXPECT_NO_THROW(renderer.Draw(camera, 5));
    }

    TEST_F(TilemapWorldRendererTest, Draw_WithParallaxFactor_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        auto* tileLayer = dynamic_cast<TilemapTileLayer*>(room->getLayersProperty()[0]);
        ASSERT_NE(tileLayer, nullptr);
        tileLayer->setParallaxFactorProperty(Vector2(0.5f, 0.5f));
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(tilemaps);

        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2(50.0f, 25.0f));

        // Exercises the non-Vector2::One World-matrix translation branch in Draw().
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapWorldRendererTest, LoadFromTilemapWorld_DoesNotThrow)
    {
        std::vector<std::unique_ptr<Tilemap>> levels;
        levels.push_back(BuildRoom("Room1", Vector2::Zero));
        TilemapWorld world(std::move(levels));

        TilemapWorldRenderer renderer(graphicsDevice);
        EXPECT_NO_THROW(renderer.Load(world));

        OrthographicCamera camera(graphicsDevice);
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapWorldRendererTest, Unload_ThenDraw_ThrowsInvalidOperationException)
    {
        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(tilemaps);
        renderer.Unload();

        OrthographicCamera camera(graphicsDevice);
        EXPECT_THROW(renderer.Draw(camera), System::InvalidOperationException);
    }

    TEST_F(TilemapWorldRendererTest, Reload_DisposesPreviousBatchesAndDoesNotThrow)
    {
        std::unique_ptr<Tilemap> room1 = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> firstLoad{room1.get()};

        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Load(firstLoad);

        std::unique_ptr<Tilemap> room2 = BuildRoom("Room2", Vector2(64.0f, 0.0f));
        std::vector<Tilemap*> secondLoad{room2.get()};

        EXPECT_NO_THROW(renderer.Load(secondLoad));

        OrthographicCamera camera(graphicsDevice);
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapWorldRendererTest, Dispose_CanBeCalledMultipleTimes)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Dispose();
        EXPECT_NO_THROW(renderer.Dispose());
    }

    TEST_F(TilemapWorldRendererTest, Dispose_PreventsSubsequentOperations)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        renderer.Dispose();

        EXPECT_THROW((void)renderer.getBlendStateProperty(), System::ObjectDisposedException);
    }

    TEST_F(TilemapWorldRendererTest, SetSamplerStateProperty_Null_ThrowsArgumentNullException)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        EXPECT_THROW(renderer.setSamplerStateProperty(nullptr), System::ArgumentNullException);
    }

    TEST_F(TilemapWorldRendererTest, SetEffectProperty_SubstitutesCustomBasicEffect)
    {
        TilemapWorldRenderer renderer(graphicsDevice);
        BasicEffect customEffect(graphicsDevice);
        customEffect.setTextureEnabledProperty(true);
        customEffect.VertexColorEnabled = true;

        renderer.setEffectProperty(customEffect);
        EXPECT_EQ(&renderer.getEffectProperty(), &customEffect);

        std::unique_ptr<Tilemap> room = BuildRoom("Room1", Vector2::Zero);
        std::vector<Tilemap*> tilemaps{room.get()};
        renderer.Load(tilemaps);

        OrthographicCamera camera(graphicsDevice);
        EXPECT_NO_THROW(renderer.Draw(camera));
    }
}
