// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapSpriteBatchRenderer.cs. Unlike
// `TilemapRenderer`/`TilemapWorldRenderer` (both blocked -- see plan.md/NEXT.md's Rendering-phase
// notes for the `DefaultEffect`/`VertexPositionColorTexture` incompatibility), this renderer draws
// exclusively through `SpriteBatch::Draw`, which has no such dependency, so it ports in full.
//
// `Tilemap tilemap`/`LoadTilemap` -> non-owning `Tilemap*` (nullable, `nullptr` = "no tilemap
// loaded", matching `_tilemap = null`); the renderer never owns the tilemap it draws, matching
// this project's established externally-owned-resource convention. `SamplerState`/`Effect`
// properties -> non-owning pointers for the same reason (`GetWrapSamplerState`'s own identity-
// comparison convention, see TilemapRendererShared.hpp, requires SamplerState be pointer-typed
// here too for consistency, though this class doesn't call GetWrapSamplerState itself -- upstream
// doesn't either, see that file's header comment). `BlendState` -> stored **by value**, matching
// `SpriteBatch::Begin`'s own by-value `BlendState` parameter (unlike `SamplerState`/`Effect`,
// `BlendState`/`GraphicsResource` are copy-constructible small state-descriptor types in this
// port, not GPU-handle owners -- see GraphicsResource.hpp).
//
// `SpriteBatch spriteBatch, OrthographicCamera camera` parameters (with
// `ArgumentNullException.ThrowIfNull`) -> taken by reference (`SpriteBatch&`, `OrthographicCamera&`),
// dropping the null checks, matching this project's established "reference-type-can't-be-null ->
// dropped null checks" convention (both types are non-copyable in this port, so pass-by-reference
// is the only viable translation regardless).
//
// The private nested `readonly record struct TileRenderPadding(Left, Top, Right, Bottom)` is kept
// as a private nested struct here (not split into its own file) since nothing outside this class
// ever references it, matching the "nested-detail-type stays nested" precedent already used
// elsewhere in this codebase for types with a single internal consumer.
#pragma once

#include "CNA/Extended/OrthographicCamera.hpp"
#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapImageLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"

#include <string>
#include <vector>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::Effect;
    using Microsoft::Xna::Framework::Graphics::SamplerState;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::SpriteSortMode;

    /**
     * @brief Tilemap renderer using SpriteBatch for simple integration with existing SpriteBatch-based code.
     *
     * This renderer draws tiles by issuing one SpriteBatch.Draw call per visible tile. Frustum
     * culling is applied per layer so only tiles within the camera's view are submitted, keeping
     * the per-frame draw call count proportional to the number of visible tiles rather than the
     * total tile count.
     *
     * Consecutive visible tile layers that share the same parallax factor are batched into a
     * single SpriteBatch.Begin/End pair to minimize state-change overhead. Layers with different
     * parallax factors each require a separate Begin/End pair.
     */
    class TilemapSpriteBatchRenderer final
    {
    public:
        TilemapSpriteBatchRenderer() = default;

        /**
         * @brief Gets/sets the blend state used when drawing tiles. Defaults to NonPremultiplied,
         * which is correct for tileset textures loaded directly from file where the PNG pixel
         * data has not been premultiplied. Set to AlphaBlend if your textures are loaded through
         * a content pipeline that premultiplies alpha by default.
         */
        [[nodiscard]] const BlendState& getBlendStateProperty() const { return blendState_; }
        void setBlendStateProperty(const BlendState& value) { blendState_ = value; }

        /**
         * @brief Gets/sets the sampler state used when drawing tiles. Defaults to PointClamp,
         * which is correct for pixel-art tilesets: clean, hard-edged tiles at any zoom level or
         * rotation angle with no edge bleeding between adjacent tiles in the texture atlas.
         * LinearClamp enables smooth sub-pixel interpolation for non-pixel-art tilesets, but
         * produces fringe artifacts at tile edges when the camera is rotated and is not
         * recommended for maps that use camera rotation.
         */
        [[nodiscard]] const SamplerState* getSamplerStateProperty() const { return samplerState_; }
        void setSamplerStateProperty(const SamplerState* value) { samplerState_ = value; }

        /**
         * @brief Gets/sets the sprite sort mode used for each SpriteBatch.Begin call. Defaults to
         * Deferred, which preserves draw order within a layer. Texture can reduce GPU state
         * changes for layers that reference tiles from multiple tilesets, at the cost of
         * rendering order guarantees.
         */
        [[nodiscard]] SpriteSortMode getSpriteSortModeProperty() const { return spriteSortMode_; }
        void setSpriteSortModeProperty(SpriteSortMode value) { spriteSortMode_ = value; }

        /** @brief Gets/sets the custom shader effect applied when drawing tiles, or nullptr to use the default. */
        [[nodiscard]] Effect* getEffectProperty() const { return effect_; }
        void setEffectProperty(Effect* value) { effect_ = value; }

        /** @brief Loads a tilemap for rendering. @throws System::ArgumentNullException @p tilemap is nullptr. */
        void LoadTilemap(Tilemap* tilemap);

        /** @brief Unloads the current tilemap. */
        void UnloadTilemap();

        /**
         * @brief Draws all visible tile layers.
         * @throws System::InvalidOperationException no tilemap is loaded.
         */
        void Draw(SpriteBatch& spriteBatch, OrthographicCamera& camera);

        /**
         * @brief Draws a single tile layer by name.
         * @throws System::Collections::Generic::KeyNotFoundException the layer name is not found.
         * @throws System::InvalidOperationException no tilemap is loaded.
         */
        void DrawLayer(SpriteBatch& spriteBatch, OrthographicCamera& camera, const std::string& layerName);

        /**
         * @brief Draws a single tile layer by index.
         * @throws System::ArgumentOutOfRangeException @p layerIndex is out of range.
         * @throws System::InvalidOperationException no tilemap is loaded.
         */
        void DrawLayer(SpriteBatch& spriteBatch, OrthographicCamera& camera, int layerIndex);

        /**
         * @brief Draws one or more tile layers by name, in the order specified. Consecutive
         * layers with the same parallax factor share a single SpriteBatch.Begin/End pair. Suited
         * for drawing groups of layers with entities interleaved between calls.
         * @throws System::Collections::Generic::KeyNotFoundException any layer name is not found.
         * @throws System::InvalidOperationException no tilemap is loaded.
         */
        void DrawLayers(SpriteBatch& spriteBatch, OrthographicCamera& camera, const std::vector<std::string>& layerNames);

        /** @brief Updates animated tiles. */
        void Update(const GameTime& gameTime);

    private:
        /** @brief Left/Top/Right/Bottom in pixels by which tile draws can overhang their owning grid cell -- see https://github.com/MonoGame-Extended/Monogame-Extended/issues/1139 */
        struct TileRenderPadding
        {
            float Left = 0.0f;
            float Top = 0.0f;
            float Right = 0.0f;
            float Bottom = 0.0f;
        };

        void DrawLayerInternal(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapLayer* layer);
        void DrawImageLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapImageLayer& imageLayer);
        void DrawObjectLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapObjectLayer& objectLayer);
        void BeginLayerBatch(SpriteBatch& spriteBatch, OrthographicCamera& camera, const Vector2& parallax);
        void DrawTileLayerCore(SpriteBatch& spriteBatch, OrthographicCamera& camera, TilemapTileLayer& tileLayer);
        [[nodiscard]] Rectangle ComputeVisibleTileRegion(OrthographicCamera& camera, TilemapTileLayer& tileLayer) const;
        [[nodiscard]] static TileRenderPadding ComputeTileRenderPadding(const Tilemap& tilemap);
        [[nodiscard]] static Color ComputeLayerColor(const TilemapLayer& layer);
        static void DrawTile(SpriteBatch& spriteBatch, Texture2D& texture, const Vector2& position, const Rectangle& sourceRect,
                              TilemapTileFlipFlags flipFlags, int tileWidth, int tileHeight, const Color& color);
        void BuildAnimatedTilesList();
        void ThrowIfNoTilemap() const;

        Tilemap* tilemap_ = nullptr;
        std::vector<TilemapTileData*> animatedTiles_;
        TileRenderPadding tileRenderPadding_;

        BlendState blendState_ = BlendState::NonPremultiplied;
        const SamplerState* samplerState_ = &SamplerState::PointClamp;
        SpriteSortMode spriteSortMode_ = SpriteSortMode::Deferred;
        Effect* effect_ = nullptr;
    };
}
