// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended.Tests's Tilemaps/Rendering/TilemapIntegrationTests.cs: real
// pixel-verification integration tests for both TilemapRenderer ("GdRenderer_*") and
// TilemapSpriteBatchRenderer ("SbRenderer_*"), covering the ground the other two test files in
// this directory (TilemapRendererTests.cpp/TilemapSpriteBatchRendererTests.cpp) explicitly do
// not: those are argument-validation/"DoesNotThrow" smoke tests, never asserting on what actually
// landed in a framebuffer.
//
// Pixel assertions are possible for the exact same reasons upstream's own header comment gives,
// verified directly against this port's own renderer .cpp files (TilemapRenderer.cpp's
// BeginDraw(), TilemapSpriteBatchRenderer.cpp's BeginLayerBatch()) rather than re-derived from
// first principles:
//  - Camera at Position=(0,0) produces an identity view matrix: OrthographicCamera::
//    GetVirtualViewMatrix's Translate(-position*parallax) * Translate(-origin) * Rotate(0) *
//    Scale(1) * Translate(origin) collapses to Translate(-origin) * Translate(origin) = Identity
//    regardless of Origin (Initialize() sets Origin to the viewport center, not (0,0), but the two
//    translations still cancel) -- and DefaultViewportAdapter::GetScaleMatrix() is Identity too.
//  - The GD renderer's projection (TilemapRenderer.cpp's BeginDraw()) is
//    Matrix::CreateOrthographicOffCenter(0, viewportW, viewportH, 0, 0, 1), which maps world
//    (x,y) directly to screen pixel (x,y). TilemapSpriteBatchRenderer.cpp's BeginLayerBatch()
//    passes the same identity view matrix straight to SpriteBatch::Begin's transformMatrix, and
//    SpriteBatch's own internal projection is the same viewport-derived orthographic matrix.
//  - Tiles placed at layer cell (0,0) render at world (0,0), so the tile center is at screen
//    pixel (tileSize/2, tileSize/2) = (16,16) for 32x32 tiles.
//  - Both renderers default to BlendState::NonPremultiplied: opacity=0 bakes vertex alpha=0, so
//    (src*0 + dst*(1-0)) leaves the clear color untouched; a fully-opaque white tile (alpha=255)
//    on a black clear produces (src*1 + dst*0) = the source color exactly, since point-sampled,
//    unlit, untinted rendering (BasicEffect with LightingEnabled=false / plain SpriteBatch.Draw)
//    does not otherwise perturb sampled texel color -- see the GD-renderer-specific note below on
//    why GD-path exact-equality assertions are legitimate despite that renderer never reading
//    TintColor (a genuine, intentionally-preserved upstream inconsistency between the two
//    renderers; see TilemapRenderer.hpp's own header comment).
//
// New test infrastructure (RenderToPixels): this project's other GraphicsDevice-backed tests
// (TilemapRendererTests.cpp, TilemapSpriteBatchRendererTests.cpp, SpriteBatchExtensionsTests.cpp)
// already established that a plain default-constructed GraphicsDevice with an explicit Viewport
// genuinely renders end-to-end headlessly in this environment (real EasyGL-over-Mesa software
// rendering), but only ever asserted "does not throw" -- never read pixels back. This file adds
// the missing off-screen render-target + readback step, following the exact idiom already
// established in cna's own examples/tests (see cna/examples/easygl_render_target_test.cpp,
// cna/examples/easygl_rt_roundtrip_test.cpp, and cna/tests/.../Texture3DTextureCubeRenderTargetTests.cpp's
// own header comment pointing at those same examples): GraphicsDevice::SetRenderTarget(&rt) ->
// draw -> GraphicsDevice::GetBackBufferData(Color*, int) (while the render target is STILL bound
// -- see RenderToPixels()'s own comment for why upstream's naive `rt.GetData(pixels)` does not
// work in this port) -> SetRenderTarget(nullptr). RenderTarget2D(GraphicsDevice&, int, int) (the
// 3-arg overload used here, matching upstream's `new RenderTarget2D(_graphicsDevice, width,
// height)`) has DepthFormat::None -- no depth attachment -- exactly like upstream's own RT
// (XNA/MonoGame's 3-arg RenderTarget2D overload defaults to DepthFormat.None too); a
// depth-buffer-less framebuffer with
// DepthStencilState::Default's depth test still enabled is well-defined (a missing depth buffer
// makes the depth test a no-op), so no extra non-upstream state changes were needed to get
// correct pixels here -- confirmed empirically by this file's own tests passing unmodified.
//
// GoogleTest fixture translation of upstream's `[Collection("GraphicsTest")]`
// GraphicsTestFixture: unlike upstream (one shared GraphicsDevice/SpriteBatch reused across the
// entire test class via xUnit collection fixture injection), this port follows this exact
// directory's own already-established idiom (TilemapRendererTests.cpp's/
// TilemapSpriteBatchRendererTests.cpp's `TilemapRendererTest`/`TilemapSpriteBatchRendererTest`
// fixtures): a fresh `GraphicsDevice graphicsDevice` member per TEST_F, viewport set in SetUp().
// `SpriteBatch` instances are constructed locally within each test body (or held as a local
// captured by the render lambda for multi-draw tests), matching that same precedent, rather than
// living on the fixture.
//
// The four upstream `[Theory]` methods parameterized over TilemapTileFlipFlags combinations
// become GoogleTest value-parameterized tests: `TilemapIntegrationFlipFlagsTest` derives from both
// `TilemapIntegrationTest` (for the shared GraphicsDevice/factory helpers) and
// `::testing::WithParamInterface<TilemapTileFlipFlags>` (the standard GoogleTest idiom for
// combining a shared non-parameterized fixture base with parameterization), with a single
// `INSTANTIATE_TEST_SUITE_P` covering all four TEST_P bodies registered under that fixture.
//
// All 28 upstream Fact/Theory test methods are ported; none were found to be unportable.
#include "CNA/Extended/Tilemaps/Rendering/TilemapRenderer.hpp"
#include "CNA/Extended/Tilemaps/Rendering/TilemapSpriteBatchRenderer.hpp"

#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileAnimation.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileAnimationFrame.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/TimeSpan.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::Texture2D;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        constexpr int kTileSize = 32;
        constexpr int kTileCenterX = kTileSize / 2; // 16
        constexpr int kTileCenterY = kTileSize / 2; // 16

        // ---- pixel helpers ----

        Color GetPixel(const std::vector<Color>& pixels, int width, int x, int y)
        {
            return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)];
        }

        bool AnyPixelNotBlack(const std::vector<Color>& pixels)
        {
            return std::any_of(pixels.begin(), pixels.end(), [](const Color& p)
                { return p.getRProperty() > 0 || p.getGProperty() > 0 || p.getBProperty() > 0; });
        }

        bool AllPixelsBlack(const std::vector<Color>& pixels)
        {
            return std::all_of(pixels.begin(), pixels.end(), [](const Color& p)
                { return p.getRProperty() == 0 && p.getGProperty() == 0 && p.getBProperty() == 0; });
        }

        std::array<Color, 4> GetExpectedQuadrantColors(TilemapTileFlipFlags flags)
        {
            const std::array<Color, 4> sourceColors{Color::Red, Color::Green, Color::Blue, Color::Yellow};
            std::array<Color, 4> destinationColors{Color::Black, Color::Black, Color::Black, Color::Black};

            for (int sourceIndex = 0; sourceIndex < static_cast<int>(sourceColors.size()); ++sourceIndex)
            {
                int x = sourceIndex % 2;
                int y = sourceIndex / 2;

                if ((flags & TilemapTileFlipFlags::FlipDiagonally) != TilemapTileFlipFlags::None)
                {
                    std::swap(x, y);
                }

                if ((flags & TilemapTileFlipFlags::FlipHorizontally) != TilemapTileFlipFlags::None)
                {
                    x = 1 - x;
                }

                if ((flags & TilemapTileFlipFlags::FlipVertically) != TilemapTileFlipFlags::None)
                {
                    y = 1 - y;
                }

                destinationColors[static_cast<std::size_t>(y * 2 + x)] = sourceColors[static_cast<std::size_t>(sourceIndex)];
            }

            return destinationColors;
        }

        void AssertColorEquals(const std::string& label, const Color& expected, const Color& actual)
        {
            EXPECT_TRUE(actual == expected)
                << label << " expected (" << static_cast<int>(expected.getRProperty()) << ", "
                << static_cast<int>(expected.getGProperty()) << ", " << static_cast<int>(expected.getBProperty())
                << ") but got (" << static_cast<int>(actual.getRProperty()) << ", " << static_cast<int>(actual.getGProperty())
                << ", " << static_cast<int>(actual.getBProperty()) << ")";
        }

        void AssertTileQuadrantsMatchExpected(const std::vector<Color>& pixels, int width, TilemapTileFlipFlags flags)
        {
            const std::array<Color, 4> expected = GetExpectedQuadrantColors(flags);
            constexpr int quarter = kTileSize / 4;
            constexpr int threeQuarter = quarter * 3;

            AssertColorEquals("top-left", expected[0], GetPixel(pixels, width, quarter, quarter));
            AssertColorEquals("top-right", expected[1], GetPixel(pixels, width, threeQuarter, quarter));
            AssertColorEquals("bottom-left", expected[2], GetPixel(pixels, width, quarter, threeQuarter));
            AssertColorEquals("bottom-right", expected[3], GetPixel(pixels, width, threeQuarter, threeQuarter));
        }

        // ---- shared fixture ----

        class TilemapIntegrationTest : public ::testing::Test
        {
        protected:
            void SetUp() override { graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480)); }

            // Renders via @p render (called with a RenderTarget2D matching the device's current
            // viewport already bound and cleared to black), then reads the whole target back.
            // Matches upstream's `RenderToPixels(Action render)` in spirit; the actual readback
            // call differs from upstream's `rt.GetData(pixels)` for a real, CNA-specific reason:
            // this port's `Texture2D::GetData` (see Texture2D.cpp) only ever returns a CPU-side
            // pixel mirror that is populated by `SetData` uploads -- rendering into a
            // RenderTarget2D via the GPU never touches that mirror, so `rt.GetData(...)` throws
            // "no CPU-side pixel data available" here (confirmed empirically: every test in this
            // file failed with exactly that exception before this fix). The correct, established
            // idiom for reading back a render target's actual rasterized GPU content in this
            // project is `GraphicsDevice::GetBackBufferData`, which performs a real backend
            // readback (EasyGL: glReadPixels) of whichever framebuffer is currently bound -- the
            // render target, while it is still bound, or the real backbuffer once unbound. This
            // matches cna's own established pattern for exactly this scenario (see
            // cna/examples/easygl_render_target_test.cpp and
            // cna/examples/easygl_rt_roundtrip_test.cpp, the latter's own comment: "Read RT1 pixel
            // while FBO is still bound"). So the readback happens BEFORE SetRenderTarget(nullptr),
            // not after, unlike upstream's ordering.
            template <typename Fn>
            std::pair<std::vector<Color>, int> RenderToPixels(Fn&& render)
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

            // ---- texture factories ----

            // A solid-color texture filled entirely with the given color.
            Texture2D* CreateFilledTexture(int width, int height, const Color& color)
            {
                textures_.push_back(std::make_unique<Texture2D>(graphicsDevice, width, height));
                Texture2D* texture = textures_.back().get();
                const std::vector<Color> data(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), color);
                texture->SetData(data.data(), static_cast<int>(data.size()));
                return texture;
            }

            // The quadrant texture is:
            //
            //   top-left = red
            //   top-right = green
            //   bottom-left = blue
            //   bottom-right = yellow
            //
            // Using four distinct corners we can sample each quadrant after rendering and tell
            // whether the diagonal, horizontal, and vertical flips were applied in the same order
            // Tiled does.
            Texture2D* CreateQuadrantTexture()
            {
                textures_.push_back(std::make_unique<Texture2D>(graphicsDevice, kTileSize, kTileSize));
                Texture2D* texture = textures_.back().get();
                std::vector<Color> data(
                    static_cast<std::size_t>(kTileSize) * static_cast<std::size_t>(kTileSize), Color::Transparent);

                for (int y = 0; y < kTileSize; ++y)
                {
                    for (int x = 0; x < kTileSize; ++x)
                    {
                        const Color color = x < kTileCenterX ? (y < kTileCenterY ? Color::Red : Color::Blue)
                                                              : (y < kTileCenterY ? Color::Green : Color::Yellow);
                        data[static_cast<std::size_t>(y) * static_cast<std::size_t>(kTileSize) + static_cast<std::size_t>(x)] = color;
                    }
                }

                texture->SetData(data.data(), static_cast<int>(data.size()));
                return texture;
            }

            // A 64x32 texture: left 32x32 = frame0Color, right 32x32 = frame1Color.
            Texture2D* CreateTwoFrameTexture(const Color& frame0Color, const Color& frame1Color)
            {
                textures_.push_back(std::make_unique<Texture2D>(graphicsDevice, kTileSize * 2, kTileSize));
                Texture2D* texture = textures_.back().get();
                std::vector<Color> data(
                    static_cast<std::size_t>(kTileSize) * 2 * static_cast<std::size_t>(kTileSize), Color::Transparent);

                for (int y = 0; y < kTileSize; ++y)
                {
                    for (int x = 0; x < kTileSize; ++x)
                    {
                        data[static_cast<std::size_t>(y) * static_cast<std::size_t>(kTileSize * 2) + static_cast<std::size_t>(x)] =
                            frame0Color; // left tile
                        data[static_cast<std::size_t>(y) * static_cast<std::size_t>(kTileSize * 2) + static_cast<std::size_t>(x) +
                             static_cast<std::size_t>(kTileSize)] = frame1Color; // right tile
                    }
                }

                texture->SetData(data.data(), static_cast<int>(data.size()));
                return texture;
            }

            // ---- tilemap factories ----

            // 10x10 tilemap with one tileset and all cells filled with a single tile.
            std::unique_ptr<Tilemap> CreateOrthogonalMap(const Color& tileColor, float opacity = 1.0f)
            {
                auto tilemap = std::make_unique<Tilemap>("OrthogonalMap", 10, 10, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                Texture2D* texture = CreateFilledTexture(kTileSize * 2, kTileSize, tileColor);
                auto tileset = std::make_unique<TilemapTileset>("Tileset", texture, kTileSize, kTileSize, 2, 2, 0, 0);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto layer = std::make_unique<TilemapTileLayer>("Ground", 10, 10, kTileSize, kTileSize);
                layer->setOpacityProperty(opacity);

                for (int y = 0; y < 10; ++y)
                {
                    for (int x = 0; x < 10; ++x)
                    {
                        layer->SetTile(x, y, TilemapTile(1));
                    }
                }

                tilemap->getLayersProperty().Add(std::move(layer));
                return tilemap;
            }

            // Tilemap with one tile at cell (0,0) using the specified flip flags.
            std::unique_ptr<Tilemap> CreateSingleTileMap(const Color& tileColor, TilemapTileFlipFlags flags)
            {
                return CreateSingleTileMap(CreateFilledTexture(kTileSize, kTileSize, tileColor), flags);
            }

            std::unique_ptr<Tilemap> CreateSingleTileMap(Texture2D* texture, TilemapTileFlipFlags flags)
            {
                auto tilemap = std::make_unique<Tilemap>("SingleTileMap", 5, 5, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                auto tileset = std::make_unique<TilemapTileset>("Tileset", texture, kTileSize, kTileSize, 1, 1, 0, 0);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto layer = std::make_unique<TilemapTileLayer>("Layer", 5, 5, kTileSize, kTileSize);
                layer->SetTile(0, 0, TilemapTile(1, flags));
                tilemap->getLayersProperty().Add(std::move(layer));
                return tilemap;
            }

            std::unique_ptr<Tilemap> CreateSingleImageCollectionTileMap(int mapWidth, int mapHeight, int tileX, int tileY,
                int imageWidth, int imageHeight, const Color& tileColor)
            {
                auto tilemap = std::make_unique<Tilemap>(
                    "ImageCollectionMap", mapWidth, mapHeight, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                auto tileset = std::make_unique<TilemapTileset>("ImageCollectionTileset", nullptr, imageWidth, imageHeight, 1, 0, 0, 0);
                tileset->setFirstGlobalIdProperty(1);

                auto tileData = std::make_unique<TilemapTileData>(0);
                tileData->setCustomImageProperty(CreateFilledTexture(imageWidth, imageHeight, tileColor));
                tileset->AddTileData(std::move(tileData));
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto layer = std::make_unique<TilemapTileLayer>("Layer", mapWidth, mapHeight, kTileSize, kTileSize);
                layer->SetTile(tileX, tileY, TilemapTile(1));
                tilemap->getLayersProperty().Add(std::move(layer));
                return tilemap;
            }

            // Tilemap with N layers, each with one tile at cell (0,0).
            std::unique_ptr<Tilemap> CreateManyLayerTilemap(int layerCount, const Color& tileColor)
            {
                auto tilemap = std::make_unique<Tilemap>("ManyLayerMap", 8, 8, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                Texture2D* texture = CreateFilledTexture(kTileSize * 2, kTileSize, tileColor);
                auto tileset = std::make_unique<TilemapTileset>("Tileset", texture, kTileSize, kTileSize, 2, 2, 0, 0);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                for (int i = 0; i < layerCount; ++i)
                {
                    auto layer = std::make_unique<TilemapTileLayer>("Layer" + std::to_string(i), 8, 8, kTileSize, kTileSize);
                    layer->SetTile(0, 0, TilemapTile(1));
                    tilemap->getLayersProperty().Add(std::move(layer));
                }

                return tilemap;
            }

            // Tilemap with 3 tile layers having different parallax factors.
            std::unique_ptr<Tilemap> CreateParallaxTilemap(const Color& tileColor)
            {
                auto tilemap = std::make_unique<Tilemap>("ParallaxMap", 10, 10, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                Texture2D* texture = CreateFilledTexture(kTileSize * 2, kTileSize, tileColor);
                auto tileset = std::make_unique<TilemapTileset>("Tileset", texture, kTileSize, kTileSize, 2, 2, 0, 0);
                tileset->setFirstGlobalIdProperty(1);
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                const std::array<float, 3> factors{0.25f, 0.5f, 1.0f};
                int index = 0;
                for (float factor : factors)
                {
                    auto layer =
                        std::make_unique<TilemapTileLayer>("Layer_" + std::to_string(index++), 10, 10, kTileSize, kTileSize);
                    layer->setParallaxFactorProperty(Vector2(factor, factor));
                    layer->SetTile(0, 0, TilemapTile(1));
                    tilemap->getLayersProperty().Add(std::move(layer));
                }

                return tilemap;
            }

            // Tilemap with a 2-frame animated tile: frame 0 = frame0Color, frame 1 = frame1Color.
            // Tile localId 0 occupies the left half of the texture; localId 1 the right half.
            std::unique_ptr<Tilemap> CreateAnimatedTilemap(const Color& frame0Color, const Color& frame1Color, float frameDuration)
            {
                auto tilemap = std::make_unique<Tilemap>("AnimatedMap", 5, 5, kTileSize, kTileSize, TilemapOrientation::Orthogonal);

                Texture2D* texture = CreateTwoFrameTexture(frame0Color, frame1Color);
                auto tileset = std::make_unique<TilemapTileset>("AnimatedTileset", texture, kTileSize, kTileSize, 2, 2, 0, 0);
                tileset->setFirstGlobalIdProperty(1);

                auto tileData = std::make_unique<TilemapTileData>(0);
                std::vector<TilemapTileAnimationFrame> frames{
                    TilemapTileAnimationFrame(0, frameDuration),
                    TilemapTileAnimationFrame(1, frameDuration),
                };
                tileData->setAnimationProperty(std::make_unique<TilemapTileAnimation>(std::move(frames)));
                tileset->AddTileData(std::move(tileData));
                tilemap->getTilesetsProperty().Add(std::move(tileset));

                auto layer = std::make_unique<TilemapTileLayer>("AnimatedLayer", 5, 5, kTileSize, kTileSize);
                layer->SetTile(0, 0, TilemapTile(1));
                tilemap->getLayersProperty().Add(std::move(layer));
                return tilemap;
            }

            GraphicsDevice graphicsDevice;
            std::vector<std::unique_ptr<Texture2D>> textures_;
        };

        // ---- flip-flag combinations shared by all 4 parameterized theories below ----

        const std::vector<TilemapTileFlipFlags> kAllFlipFlagCombinations{
            TilemapTileFlipFlags::None,
            TilemapTileFlipFlags::FlipHorizontally,
            TilemapTileFlipFlags::FlipVertically,
            TilemapTileFlipFlags::FlipHorizontally | TilemapTileFlipFlags::FlipVertically,
            TilemapTileFlipFlags::FlipDiagonally,
            TilemapTileFlipFlags::FlipDiagonally | TilemapTileFlipFlags::FlipHorizontally,
            TilemapTileFlipFlags::FlipDiagonally | TilemapTileFlipFlags::FlipVertically,
            TilemapTileFlipFlags::FlipDiagonally | TilemapTileFlipFlags::FlipHorizontally | TilemapTileFlipFlags::FlipVertically,
        };

        class TilemapIntegrationFlipFlagsTest : public TilemapIntegrationTest,
                                                 public ::testing::WithParamInterface<TilemapTileFlipFlags>
        {
        };
    }

    // ---- Tiles are drawn ----

    TEST_F(TilemapIntegrationTest, GdRenderer_OrthogonalMap_WithTilesPlaced_DrawsPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        EXPECT_EQ(Color::White, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_OrthogonalMap_WithTilesPlaced_DrawsPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        EXPECT_EQ(Color::White, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    // ---- Layer visibility ----

    TEST_F(TilemapIntegrationTest, GdRenderer_HiddenLayer_DrawsNoPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        tilemap->getLayersProperty()[0]->setIsVisibleProperty(false);
        renderer.LoadTilemap(tilemap.get());

        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        EXPECT_EQ(Color::Black, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_HiddenLayer_DrawsNoPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());
        tilemap->getLayersProperty()[0]->setIsVisibleProperty(false);

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        EXPECT_EQ(Color::Black, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    // ---- Opacity ----

    TEST_F(TilemapIntegrationTest, GdRenderer_ZeroOpacity_DrawsNoPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        // Opacity must be set before LoadTilemap: GD renderer bakes opacity into vertex alpha.
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White, 0.0f);
        renderer.LoadTilemap(tilemap.get());

        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        EXPECT_EQ(Color::Black, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_ZeroOpacity_DrawsNoPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White, 0.0f);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        EXPECT_EQ(Color::Black, GetPixel(pixels, width, kTileCenterX, kTileCenterY));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_PartialOpacity_ProducesDifferentPixelThanFullOpacity)
    {
        TilemapSpriteBatchRenderer renderer;
        SpriteBatch spriteBatch(graphicsDevice);

        std::unique_ptr<Tilemap> fullyOpaque = CreateOrthogonalMap(Color::White, 1.0f);
        renderer.LoadTilemap(fullyOpaque.get());
        const auto [fullPixels, fullWidth] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        std::unique_ptr<Tilemap> halfOpacity = CreateOrthogonalMap(Color::White, 0.5f);
        renderer.LoadTilemap(halfOpacity.get());
        const auto [halfPixels, halfWidth] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        const Color fullPixel = GetPixel(fullPixels, fullWidth, kTileCenterX, kTileCenterY);
        const Color halfPixel = GetPixel(halfPixels, halfWidth, kTileCenterX, kTileCenterY);

        EXPECT_NE(fullPixel, halfPixel);
    }

    // ---- Tint color (SpriteBatch only; GD renderer bakes opacity but not tint) ----

    TEST_F(TilemapIntegrationTest, SbRenderer_RedTintOnWhiteTile_RendersRedPixel)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        tilemap->getLayersProperty()[0]->setTintColorProperty(Color::Red);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        const Color pixel = GetPixel(pixels, width, kTileCenterX, kTileCenterY);
        EXPECT_GT(pixel.getRProperty(), 200) << "Expected high red component, got R=" << static_cast<int>(pixel.getRProperty());
        EXPECT_LT(pixel.getGProperty(), 10) << "Expected near-zero green component, got G=" << static_cast<int>(pixel.getGProperty());
        EXPECT_LT(pixel.getBProperty(), 10) << "Expected near-zero blue component, got B=" << static_cast<int>(pixel.getBProperty());
    }

    // ---- Camera transformations ----

    TEST_F(TilemapIntegrationTest, GdRenderer_CameraViewingTiles_DrawsPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.Draw(camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    TEST_F(TilemapIntegrationTest, GdRenderer_CameraFarFromTiles_DrawsNoPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               camera.LookAt(Vector2(100000.0f, 100000.0f));
                                               renderer.Draw(camera);
                                           }).first;

        EXPECT_TRUE(AllPixelsBlack(pixels));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_CameraViewingTiles_DrawsPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.Draw(spriteBatch, camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_CameraFarFromTiles_DrawsNoPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               camera.LookAt(Vector2(100000.0f, 100000.0f));
                                               renderer.Draw(spriteBatch, camera);
                                           }).first;

        EXPECT_TRUE(AllPixelsBlack(pixels));
    }

    TEST_F(TilemapIntegrationTest, GdRenderer_ZoomedInCamera_DrawsPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               camera.ZoomIn(2.0f);
                                               renderer.Draw(camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_ZoomedInCamera_DrawsPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateOrthogonalMap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               camera.ZoomIn(2.0f);
                                               renderer.Draw(spriteBatch, camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    // ---- Animation frame advancement ----

    TEST_F(TilemapIntegrationTest, SbRenderer_AnimationFrameAdvancement_PixelColorChanges)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateAnimatedTilemap(Color::Red, Color::Blue, 0.1f);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [frame0Pixels, frame0Width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        renderer.Update(GameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.15)));

        const auto [frame1Pixels, frame1Width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        const Color before = GetPixel(frame0Pixels, frame0Width, kTileCenterX, kTileCenterY);
        const Color after = GetPixel(frame1Pixels, frame1Width, kTileCenterX, kTileCenterY);

        // Frame 0 = red; frame 1 = blue. Colors must be visibly different.
        EXPECT_GT(before.getRProperty(), 200) << "Expected red pixel before animation advance, got R="
                                               << static_cast<int>(before.getRProperty());
        EXPECT_GT(after.getBProperty(), 200) << "Expected blue pixel after animation advance, got B="
                                              << static_cast<int>(after.getBProperty());
        EXPECT_NE(before, after);
    }

    TEST_F(TilemapIntegrationTest, GdRenderer_AnimationFrameAdvancement_PixelColorChanges)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateAnimatedTilemap(Color::Red, Color::Blue, 0.1f);
        renderer.LoadTilemap(tilemap.get());

        const auto [frame0Pixels, frame0Width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        // Update advances the animation and marks the VBO dirty.
        renderer.Update(GameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.15)));

        // The second draw rebuilds the VBO with the new frame's UV coordinates.
        const auto [frame1Pixels, frame1Width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        const Color before = GetPixel(frame0Pixels, frame0Width, kTileCenterX, kTileCenterY);
        const Color after = GetPixel(frame1Pixels, frame1Width, kTileCenterX, kTileCenterY);

        EXPECT_GT(before.getRProperty(), 200) << "Expected red pixel before animation advance, got R="
                                               << static_cast<int>(before.getRProperty());
        EXPECT_GT(after.getBProperty(), 200) << "Expected blue pixel after animation advance, got B="
                                              << static_cast<int>(after.getBProperty());
        EXPECT_NE(before, after);
    }

    // ---- All flip flag combinations ----

    TEST_P(TilemapIntegrationFlipFlagsTest, SbRenderer_AllFlipFlagCombinations_TileIsDrawn)
    {
        const TilemapTileFlipFlags flags = GetParam();
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateSingleTileMap(Color::White, flags);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.Draw(spriteBatch, camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels)) << "Expected tile to be drawn for flip flags " << static_cast<int>(flags);
    }

    TEST_P(TilemapIntegrationFlipFlagsTest, GdRenderer_AllFlipFlagCombinations_TileIsDrawn)
    {
        const TilemapTileFlipFlags flags = GetParam();
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSingleTileMap(Color::White, flags);
        renderer.LoadTilemap(tilemap.get());

        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.Draw(camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels)) << "Expected tile to be drawn for flip flags " << static_cast<int>(flags);
    }

    // Added while investigating issue #1138:
    // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1138
    // These assertions verify the exact orientation Tiled expects for every
    // diagonal/horizontal/vertical flag combination, not just that some pixels draw.
    TEST_P(TilemapIntegrationFlipFlagsTest, SbRenderer_AllFlipFlagCombinations_MatchTiledOrientation)
    {
        const TilemapTileFlipFlags flags = GetParam();
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateSingleTileMap(CreateQuadrantTexture(), flags);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        AssertTileQuadrantsMatchExpected(pixels, width, flags);
    }

    // Added while investigating issue #1138:
    // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1138
    // This mirrors the SpriteBatch test above so both renderers are held to the
    // same Tiled orientation rules for diagonal flip combinations.
    TEST_P(TilemapIntegrationFlipFlagsTest, GdRenderer_AllFlipFlagCombinations_MatchTiledOrientation)
    {
        const TilemapTileFlipFlags flags = GetParam();
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateSingleTileMap(CreateQuadrantTexture(), flags);
        renderer.LoadTilemap(tilemap.get());

        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(camera);
        });

        AssertTileQuadrantsMatchExpected(pixels, width, flags);
    }

    INSTANTIATE_TEST_SUITE_P(TilemapIntegration, TilemapIntegrationFlipFlagsTest, ::testing::ValuesIn(kAllFlipFlagCombinations));

    // ---- Vic's scenario: many layers with groups ----

    TEST_F(TilemapIntegrationTest, GdRenderer_ManyLayersWithGroups_DrawsPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateManyLayerTilemap(30, Color::White);
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("Background", 0, 10);
        renderer.DefineLayerGroup("Midground", 10, 10);
        renderer.DefineLayerGroup("Foreground", 20, 10);

        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.BeginDraw(camera);
                                               renderer.DrawLayerGroup("Background");
                                               renderer.DrawLayerGroup("Midground");
                                               renderer.DrawLayerGroup("Foreground");
                                               renderer.EndDraw();
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    // ---- Dynamic group changes at runtime ----

    TEST_F(TilemapIntegrationTest, GdRenderer_DynamicGroupChanges_DrawsPixelsAfterEachChange)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateManyLayerTilemap(6, Color::White);
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("GroupA", 0, 3);
        renderer.DefineLayerGroup("GroupB", 3, 3);
        const std::vector<Color> pixels1 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels1));

        renderer.DefineLayerGroup("GroupA", 0, 2);
        renderer.DefineLayerGroup("GroupB", 2, 4);
        const std::vector<Color> pixels2 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels2));

        renderer.RemoveLayerGroup("GroupA");
        const std::vector<Color> pixels3 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels3));
    }

    // ---- Mixed GD renderer + SpriteBatch interleaving ----

    TEST_F(TilemapIntegrationTest, GdRenderer_InterleavedWithSpriteBatch_DrawsPixels)
    {
        TilemapRenderer renderer(graphicsDevice);
        std::unique_ptr<Tilemap> tilemap = CreateManyLayerTilemap(4, Color::White);
        renderer.LoadTilemap(tilemap.get());

        renderer.DefineLayerGroup("Background", 0, 2);
        renderer.DefineLayerGroup("Foreground", 2, 2);

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.BeginDraw(camera);
                                               renderer.DrawLayerGroup("Background");

                                               renderer.SaveGraphicsDeviceState();
                                               spriteBatch.Begin();
                                               spriteBatch.End();
                                               renderer.RestoreGraphicsDeviceState();

                                               renderer.DrawLayerGroup("Foreground");
                                               renderer.EndDraw();
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    // ---- LoadTilemap / UnloadTilemap / reload cycle ----

    TEST_F(TilemapIntegrationTest, GdRenderer_ReloadCycle_DrawsPixelsBothTimes)
    {
        TilemapRenderer renderer(graphicsDevice);

        std::unique_ptr<Tilemap> firstMap = CreateOrthogonalMap(Color::White);
        std::unique_ptr<Tilemap> secondMap = CreateManyLayerTilemap(3, Color::White);

        renderer.LoadTilemap(firstMap.get());
        const std::vector<Color> pixels1 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels1));

        renderer.UnloadTilemap();

        renderer.LoadTilemap(secondMap.get());
        const std::vector<Color> pixels2 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels2));
    }

    TEST_F(TilemapIntegrationTest, SbRenderer_ReloadCycle_DrawsPixelsBothTimes)
    {
        TilemapSpriteBatchRenderer renderer;

        std::unique_ptr<Tilemap> firstMap = CreateOrthogonalMap(Color::White);
        std::unique_ptr<Tilemap> secondMap = CreateManyLayerTilemap(3, Color::White);

        SpriteBatch spriteBatch(graphicsDevice);

        renderer.LoadTilemap(firstMap.get());
        const std::vector<Color> pixels1 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(spriteBatch, camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels1));

        renderer.UnloadTilemap();

        renderer.LoadTilemap(secondMap.get());
        const std::vector<Color> pixels2 = RenderToPixels([&]() {
                                                OrthographicCamera camera(graphicsDevice);
                                                renderer.Draw(spriteBatch, camera);
                                            }).first;
        EXPECT_TRUE(AnyPixelNotBlack(pixels2));
    }

    // ---- Parallax layers ----

    TEST_F(TilemapIntegrationTest, SbRenderer_ParallaxLayers_DrawsPixels)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap = CreateParallaxTilemap(Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Color> pixels = RenderToPixels([&]() {
                                               OrthographicCamera camera(graphicsDevice);
                                               renderer.Draw(spriteBatch, camera);
                                           }).first;

        EXPECT_TRUE(AnyPixelNotBlack(pixels));
    }

    // Added while investigating issue #1139:
    // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1139
    TEST_F(TilemapIntegrationTest, SbRenderer_WideImageCollectionTile_RemainsVisibleAtLeftEdge)
    {
        TilemapSpriteBatchRenderer renderer;
        std::unique_ptr<Tilemap> tilemap =
            CreateSingleImageCollectionTileMap(4, 4, 0, 0, kTileSize * 2, kTileSize, Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            camera.setPositionProperty(Vector2(kTileSize + kTileSize / 2.0f, 0.0f));
            renderer.Draw(spriteBatch, camera);
        });

        EXPECT_EQ(Color::White, GetPixel(pixels, width, 8, kTileCenterY));
    }

    // Added while investigating issue #1139:
    // https://github.com/MonoGame-Extended/Monogame-Extended/issues/1139
    TEST_F(TilemapIntegrationTest, SbRenderer_TallImageCollectionTile_RemainsVisibleAtBottomEdge)
    {
        TilemapSpriteBatchRenderer renderer;
        const int viewportHeight = graphicsDevice.getViewportProperty().getHeightProperty();
        const int tileY = static_cast<int>(std::ceil(static_cast<float>(viewportHeight) / static_cast<float>(kTileSize)));

        std::unique_ptr<Tilemap> tilemap =
            CreateSingleImageCollectionTileMap(4, tileY + 2, 0, tileY, kTileSize, kTileSize * 2, Color::White);
        renderer.LoadTilemap(tilemap.get());

        SpriteBatch spriteBatch(graphicsDevice);
        const auto [pixels, width] = RenderToPixels([&]() {
            OrthographicCamera camera(graphicsDevice);
            renderer.Draw(spriteBatch, camera);
        });

        const int height = static_cast<int>(pixels.size()) / width;
        EXPECT_EQ(Color::White, GetPixel(pixels, width, kTileCenterX, height - 8));
    }
}
