// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended.Tests's Tilemaps/Rendering/TilemapRendererTests.cs, adapted for
// GoogleTest and this port's reference-parameter conventions (Constructor(GraphicsDevice&),
// BeginDraw/Draw/DrawLayers(OrthographicCamera&), Update(const GameTime&) all take non-null
// references, so the corresponding upstream "...WithNull...ThrowsArgumentNullException" tests have
// no reachable C++ equivalent and are dropped -- see TilemapRenderer.hpp's own header comment).
// `LoadTilemap(Tilemap*)` keeps its null check (matching TilemapSpriteBatchRendererTests.cpp's
// identical precedent), so that one test IS ported.
//
// Extended with fresh tests beyond upstream's coverage, specifically real headless
// GraphicsDevice-backed draw calls exercising the DefaultEffect -> BasicEffect substitution this
// port makes (see TilemapRenderer.hpp's top comment): merged/separate tile layers, a non-repeating
// image layer, a REPEATING image layer (the DynamicVertexBuffer + wrap-sampler
// RepeatImageLayerModel path -- the most novel code in this port), an object layer, and layer
// groups -- following the same real-EasyGL-over-Mesa headless rendering pattern already
// established in TilemapSpriteBatchRendererTests.cpp/SpriteBatchExtensionsTests.cpp.
#include "CNA/Extended/Tilemaps/Rendering/TilemapRenderer.hpp"

#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapImageLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "System/ArgumentException.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Collections/Generic/KeyNotFoundException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
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
        class TilemapRendererTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480));
            }

            std::unique_ptr<Tilemap> CreateSimpleTilemap()
            {
                auto tilemap = std::make_unique<Tilemap>("TestMap", 10, 10, 32, 32, TilemapOrientation::Orthogonal);

                auto layer = std::make_unique<TilemapTileLayer>("TestLayer", 10, 10, 32, 32);
                tilemap->getLayersProperty().Add(std::move(layer));

                textures.push_back(std::make_unique<Texture2D>(graphicsDevice, 32, 32));
                auto tileset = std::make_unique<TilemapTileset>("TestTileset", textures.back().get(), 32, 32, 10, 10);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                return tilemap;
            }

            // 4 layers ("Layer1".."Layer4"), 5x5 tiles, sharing one 2x2-tile 64x64px tileset.
            std::unique_ptr<Tilemap> CreateMultiLayerTilemap()
            {
                auto tilemap = std::make_unique<Tilemap>("MultiLayerMap", 5, 5, 32, 32, TilemapOrientation::Orthogonal);

                textures.push_back(std::make_unique<Texture2D>(graphicsDevice, 64, 64));
                auto tileset = std::make_unique<TilemapTileset>("TestTileset", textures.back().get(), 32, 32, 4, 2);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                for (int i = 1; i <= 4; ++i)
                {
                    auto layer = std::make_unique<TilemapTileLayer>("Layer" + std::to_string(i), 5, 5, 32, 32);
                    layer->SetTile(0, 0, TilemapTile(1));
                    layer->SetTile(1, 0, TilemapTile(2));
                    tilemap->getLayersProperty().Add(std::move(layer));
                }

                return tilemap;
            }

            GraphicsDevice graphicsDevice;
            std::vector<std::unique_ptr<Texture2D>> textures;
        };
    }

    TEST_F(TilemapRendererTest, LoadTilemap_WithNullTilemap_ThrowsArgumentNullException)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_THROW(renderer.LoadTilemap(nullptr), System::ArgumentNullException);
    }

    TEST_F(TilemapRendererTest, SetDefaultRenderMode_ChangesMode)
    {
        TilemapRenderer renderer(graphicsDevice);
        renderer.SetDefaultRenderMode(RenderMode::Separate);
        EXPECT_EQ(renderer.getDefaultRenderModeProperty(), RenderMode::Separate);
    }

    TEST_F(TilemapRendererTest, DefaultProperties_MatchUpstreamDefaults)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_EQ(renderer.getDefaultRenderModeProperty(), RenderMode::Merged);
        EXPECT_EQ(renderer.getSamplerStateProperty(), &SamplerState::PointClamp);
        EXPECT_EQ(renderer.getBlendStateProperty().getColorSourceBlendProperty(),
                  BlendState::NonPremultiplied.getColorSourceBlendProperty());
        EXPECT_TRUE(renderer.getLayerGroupsProperty().empty());
    }

    TEST_F(TilemapRendererTest, BeginDraw_WithoutLoadedTilemap_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.BeginDraw(camera), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, BeginDraw_CalledTwice_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        renderer.BeginDraw(camera);
        EXPECT_THROW(renderer.BeginDraw(camera), System::InvalidOperationException);
        renderer.EndDraw();
    }

    TEST_F(TilemapRendererTest, EndDraw_WithoutBeginDraw_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.EndDraw(), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, DrawLayer_ByName_WithInvalidName_ThrowsKeyNotFoundException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        renderer.BeginDraw(camera);
        EXPECT_THROW(renderer.DrawLayer(std::string("NonExistentLayer")), System::Collections::Generic::KeyNotFoundException);
        renderer.EndDraw();
    }

    TEST_F(TilemapRendererTest, DrawLayer_ByIndex_WithInvalidIndex_ThrowsArgumentOutOfRangeException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        renderer.BeginDraw(camera);
        EXPECT_THROW(renderer.DrawLayer(-1), System::ArgumentOutOfRangeException);
        EXPECT_THROW(renderer.DrawLayer(999), System::ArgumentOutOfRangeException);
        renderer.EndDraw();
    }

    TEST_F(TilemapRendererTest, DrawLayer_WithoutBeginDraw_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.DrawLayer(0), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, Draw_WithoutLoadedTilemap_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        OrthographicCamera camera(graphicsDevice);

        EXPECT_THROW(renderer.Draw(camera), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, HasLayerGroup_WithNonExistentGroup_ReturnsFalse)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_FALSE(renderer.HasLayerGroup("NonExistentGroup"));
    }

    TEST_F(TilemapRendererTest, Dispose_CanBeCalledMultipleTimes)
    {
        TilemapRenderer renderer(graphicsDevice);
        renderer.Dispose();
        EXPECT_NO_THROW(renderer.Dispose());
    }

    TEST_F(TilemapRendererTest, Dispose_PreventsSubsequentOperations)
    {
        TilemapRenderer renderer(graphicsDevice);
        renderer.Dispose();

        EXPECT_THROW((void)renderer.getDefaultRenderModeProperty(), System::ObjectDisposedException);
    }

    // --- DefineLayerGroup (by name) ---

    TEST_F(TilemapRendererTest, DefineLayerGroup_WithInvalidLayerName_ThrowsKeyNotFoundException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.DefineLayerGroup("Group", {"DoesNotExist"}), System::Collections::Generic::KeyNotFoundException);
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_WithValidNames_CreatesGroup)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("Background", {"Layer1", "Layer2"});

        EXPECT_TRUE(renderer.HasLayerGroup("Background"));
        const std::vector<std::string> groups = renderer.getLayerGroupsProperty();
        EXPECT_NE(std::find(groups.begin(), groups.end(), "Background"), groups.end());
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_WithSameName_RedefinesGroup)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("Group", {"Layer1", "Layer2"});
        renderer.DefineLayerGroup("Group", {"Layer3", "Layer4"});

        EXPECT_TRUE(renderer.HasLayerGroup("Group"));
        EXPECT_EQ(renderer.getLayerGroupsProperty().size(), 1u);
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_LayerMovedFromExistingGroup_EnforcesExclusiveMembership)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("GroupA", {"Layer1", "Layer2"});
        renderer.DefineLayerGroup("GroupB", {"Layer2", "Layer3"});

        // Layer2 was moved to GroupB; both groups must still exist.
        EXPECT_TRUE(renderer.HasLayerGroup("GroupA"));
        EXPECT_TRUE(renderer.HasLayerGroup("GroupB"));
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_WithoutLoadedTilemap_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_THROW(renderer.DefineLayerGroup("Group", {"Layer1"}), System::InvalidOperationException);
    }

    // --- DefineLayerGroup (by index) ---

    TEST_F(TilemapRendererTest, DefineLayerGroup_ByIndex_WithValidRange_CreatesGroup)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("Background", 0, 2);

        EXPECT_TRUE(renderer.HasLayerGroup("Background"));
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_ByIndex_WithNegativeStart_ThrowsArgumentOutOfRangeException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.DefineLayerGroup("Group", -1, 2), System::ArgumentOutOfRangeException);
    }

    TEST_F(TilemapRendererTest, DefineLayerGroup_ByIndex_WithCountExceedingBounds_ThrowsArgumentOutOfRangeException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.DefineLayerGroup("Group", 3, 99), System::ArgumentOutOfRangeException);
    }

    // --- RemoveLayerGroup ---

    TEST_F(TilemapRendererTest, RemoveLayerGroup_ExistingGroup_RemovesGroup)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        renderer.DefineLayerGroup("Background", {"Layer1", "Layer2"});

        renderer.RemoveLayerGroup("Background");

        EXPECT_FALSE(renderer.HasLayerGroup("Background"));
    }

    TEST_F(TilemapRendererTest, RemoveLayerGroup_NonExistentGroup_DoesNotThrow)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_NO_THROW(renderer.RemoveLayerGroup("DoesNotExist"));
    }

    // --- MarkGroupDirty ---

    TEST_F(TilemapRendererTest, MarkGroupDirty_NonExistentGroup_ThrowsArgumentException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());

        EXPECT_THROW(renderer.MarkGroupDirty("DoesNotExist"), System::ArgumentException);
    }

    // --- DrawLayerGroup ---

    TEST_F(TilemapRendererTest, DrawLayerGroup_WithoutBeginDraw_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        renderer.DefineLayerGroup("Background", {"Layer1", "Layer2"});

        EXPECT_THROW(renderer.DrawLayerGroup("Background"), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, DrawLayerGroup_WithNonExistentGroup_ThrowsArgumentException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        renderer.BeginDraw(camera);
        EXPECT_THROW(renderer.DrawLayerGroup("DoesNotExist"), System::ArgumentException);
        renderer.EndDraw();
    }

    // --- RenderMode / real headless draws ---

    TEST_F(TilemapRendererTest, Draw_InMergedMode_AfterRemoveLayerGroup_RebuildsMergedBuffer)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        renderer.SetDefaultRenderMode(RenderMode::Merged);
        renderer.DefineLayerGroup("Background", {"Layer1", "Layer2"});
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(camera));

        renderer.RemoveLayerGroup("Background");

        // After removing the group, Layer1 and Layer2 return to ungrouped. A second draw must
        // rebuild the merged buffer without throwing.
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, SwitchingRenderMode_BetweenMergedAndSeparate_Succeeds)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        renderer.SetDefaultRenderMode(RenderMode::Separate);
        EXPECT_NO_THROW(renderer.Draw(camera));

        renderer.SetDefaultRenderMode(RenderMode::Merged);
        EXPECT_NO_THROW(renderer.Draw(camera));

        renderer.SetDefaultRenderMode(RenderMode::Separate);
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, Draw_InMergedMode_WithAllLayersGrouped_Succeeds)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        renderer.SetDefaultRenderMode(RenderMode::Merged);
        renderer.DefineLayerGroup("All", {"Layer1", "Layer2", "Layer3", "Layer4"});
        OrthographicCamera camera(graphicsDevice);

        // All layers are grouped; the merged ungrouped buffer will be empty.
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, Draw_WithLoadedTileLayer_DoesNotThrow)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        auto* tileLayer = dynamic_cast<TilemapTileLayer*>(tilemap->getLayersProperty()[0]);
        ASSERT_NE(tileLayer, nullptr);
        tileLayer->SetTile(0, 0, TilemapTile(1));
        tileLayer->SetTile(1, 0, TilemapTile(2, TilemapTileFlipFlags::FlipHorizontally));

        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, Draw_WithImageLayer_DoesNotThrow)
    {
        auto tilemap = std::make_unique<Tilemap>("ImgMap", 4, 4, 32, 32, TilemapOrientation::Orthogonal);
        textures.push_back(std::make_unique<Texture2D>(graphicsDevice, 16, 16));
        auto imageLayer = std::make_unique<TilemapImageLayer>("Background", textures.back().get(), Vector2::Zero);
        tilemap->getLayersProperty().Add(std::move(imageLayer));

        TilemapRenderer renderer(graphicsDevice);
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    // Exercises the DynamicVertexBuffer + wrap-SamplerState RepeatImageLayerModel path -- the
    // most novel code this port's BasicEffect substitution had to get right (see
    // TilemapRenderer.hpp's own header comment).
    TEST_F(TilemapRendererTest, Draw_WithRepeatingImageLayer_DoesNotThrow)
    {
        auto tilemap = std::make_unique<Tilemap>("RepeatMap", 4, 4, 32, 32, TilemapOrientation::Orthogonal);
        textures.push_back(std::make_unique<Texture2D>(graphicsDevice, 16, 16));
        auto imageLayer = std::make_unique<TilemapImageLayer>("Background", textures.back().get(), Vector2::Zero);
        imageLayer->setRepeatXProperty(true);
        imageLayer->setRepeatYProperty(true);
        tilemap->getLayersProperty().Add(std::move(imageLayer));

        TilemapRenderer renderer(graphicsDevice);
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);
        camera.setPositionProperty(Vector2(37.0f, 19.0f));

        EXPECT_NO_THROW(renderer.Draw(camera));
        // Draw a second time: exercises UpdateVertices()'s per-frame re-upload path with the
        // camera having already been positioned once.
        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, Draw_WithObjectLayer_DoesNotThrow)
    {
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        auto objectLayer = std::make_unique<TilemapObjectLayer>("Objects");
        objectLayer->AddObject(std::make_unique<TilemapTileObject>(1, Vector2(10, 10), TilemapTile(1), Vector2(32, 32)));
        tilemap->getLayersProperty().Add(std::move(objectLayer));

        TilemapRenderer renderer(graphicsDevice);
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }

    TEST_F(TilemapRendererTest, DrawLayers_WithMultipleNames_DoesNotThrow)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateMultiLayerTilemap();
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        const std::vector<std::string> names{"Layer1", "Layer2"};
        EXPECT_NO_THROW(renderer.DrawLayers(camera, names));
    }

    TEST_F(TilemapRendererTest, UnloadTilemap_ThenDraw_ThrowsInvalidOperationException)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());
        renderer.UnloadTilemap();

        OrthographicCamera camera(graphicsDevice);
        EXPECT_THROW(renderer.Draw(camera), System::InvalidOperationException);
    }

    TEST_F(TilemapRendererTest, Update_WithNoAnimatedTiles_DoesNotThrow)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        renderer.LoadTilemap(tilemap.get());

        Microsoft::Xna::Framework::GameTime gameTime;
        EXPECT_NO_THROW(renderer.Update(gameTime));
    }

    // --- Effect / SamplerState substitution ---

    TEST_F(TilemapRendererTest, SetSamplerStateProperty_Null_ThrowsArgumentNullException)
    {
        TilemapRenderer renderer(graphicsDevice);
        EXPECT_THROW(renderer.setSamplerStateProperty(nullptr), System::ArgumentNullException);
    }

    TEST_F(TilemapRendererTest, SetEffectProperty_SubstitutesCustomBasicEffect)
    {
        TilemapRenderer renderer(graphicsDevice);
        BasicEffect customEffect(graphicsDevice);
        customEffect.setTextureEnabledProperty(true);
        customEffect.VertexColorEnabled = true;

        renderer.setEffectProperty(customEffect);

        EXPECT_EQ(&renderer.getEffectProperty(), &customEffect);

        // The substituted effect must actually be usable for a real draw.
        std::unique_ptr<Tilemap> tilemap = CreateSimpleTilemap();
        auto* tileLayer = dynamic_cast<TilemapTileLayer*>(tilemap->getLayersProperty()[0]);
        ASSERT_NE(tileLayer, nullptr);
        tileLayer->SetTile(0, 0, TilemapTile(1));
        renderer.LoadTilemap(tilemap.get());
        OrthographicCamera camera(graphicsDevice);

        EXPECT_NO_THROW(renderer.Draw(camera));
    }
}
