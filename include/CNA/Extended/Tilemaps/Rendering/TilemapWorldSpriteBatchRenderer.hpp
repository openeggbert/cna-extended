// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapWorldSpriteBatchRenderer.cs.
// Unblocked for the same reason as its sibling TilemapSpriteBatchRenderer -- see that header's
// comment; this renderer also draws exclusively through SpriteBatch::Draw.
//
// `IEnumerable<Tilemap> tilemaps` -> `const std::vector<Tilemap*>&` (non-owning: the renderer
// never owns the tilemaps it bakes, matching TilemapSpriteBatchRenderer's `Tilemap*` convention).
//
// `Dictionary<WorldGroupKey, WorldGroup>`/`Dictionary<WorldRoomKey, WorldRoomBatch>` (both local
// to `Load`, used purely for lookup-or-create grouping during the one-time bake pass) -> linear
// `std::vector<std::pair<Key, Pointer>>` scans instead of a hash map, avoiding the need for a
// `std::hash` specialization over a `Vector2`-containing key; load-time group/room counts are
// small (bounded by distinct (depth, parallax[, tilemap]) combinations, not tile count), so O(n)
// lookup here is not a real performance concern, unlike the per-tile Draw-time hot path.
// `HashSet<TilemapTileData>` (dedup for `_animatedTiles`) -> `std::unordered_set<const
// TilemapTileData*>`, which needs no custom hash (raw pointers hash natively).
//
// `List<WorldGroup> _worldGroups` / `WorldGroup.Rooms : List<WorldRoomBatch>` (both hold GC
// references, kept stable as more items are added) -> `std::vector<std::unique_ptr<WorldGroup>>`/
// `std::vector<std::unique_ptr<WorldRoomBatch>>`: `Load`'s local group/room index stores raw
// pointers into these entries *while still adding more entries* (across the whole tilemap/layer
// loop) -- a plain `std::vector<WorldGroup>` would invalidate those pointers on reallocation,
// exactly the "reference stability across incremental appends" hazard already solved this way for
// `TilemapLayerCollection`/`EntityManager` elsewhere in this project. `WorldRoomBatch.Tiles :
// List<WorldTileSprite>` has no such aliasing concern (nothing holds a pointer into it across
// appends), so it stays a plain `std::vector<WorldTileSprite>`.
//
// The three private nested types (`WorldGroupKey`/`WorldRoomKey`, `WorldTileSprite`,
// `WorldRoomBatch`, `WorldGroup`) are declared entirely inside the .cpp (not this header) since
// none of them appear in any public signature -- unlike `TilemapSpriteBatchRenderer`'s
// `TileRenderPadding` (also private, but needed in the header because it's a private *member's*
// type, so the class definition itself requires it complete). Here, `worldGroups_`'s element type
// is `std::unique_ptr<WorldGroup>`, and a `unique_ptr`'s declaration only needs its pointee
// forward-declared, not complete, so `WorldGroup` can be forward-declared here and defined only
// in the .cpp, keeping this header's public surface uncluttered by definitions callers cannot see
// or use.
#pragma once

#include "CNA/Extended/OrthographicCamera.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"
#include "CNA/Extended/Tilemaps/TilemapWorld.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/Effect.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"

#include <memory>
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
     * @brief World-map tilemap renderer using SpriteBatch for simple integration with existing SpriteBatch-based code.
     *
     * Loads a collection of tilemaps and bakes per-tile draw data into world space at load time,
     * grouped by (WorldDepth, parallax factor). At draw time one SpriteBatch.Begin/End pair is
     * issued per unique parallax factor, with room-level and per-tile AABB culling applied within
     * each batch.
     *
     * Animated tiles are supported. Call Update() each frame to advance animation state.
     *
     * @see TilemapWorldRenderer for a higher-performance GraphicsDevice-based alternative --
     * note it does not support animated tiles in world mode, unlike this renderer.
     * @see TilemapSpriteBatchRenderer, the single-tilemap counterpart this shares its
     * SpriteBatch-based drawing approach with.
     */
    class TilemapWorldSpriteBatchRenderer final
    {
    public:
        TilemapWorldSpriteBatchRenderer();
        ~TilemapWorldSpriteBatchRenderer();
        TilemapWorldSpriteBatchRenderer(const TilemapWorldSpriteBatchRenderer&) = delete;
        TilemapWorldSpriteBatchRenderer& operator=(const TilemapWorldSpriteBatchRenderer&) = delete;
        TilemapWorldSpriteBatchRenderer(TilemapWorldSpriteBatchRenderer&&) noexcept;
        TilemapWorldSpriteBatchRenderer& operator=(TilemapWorldSpriteBatchRenderer&&) noexcept;

        /**
         * @brief Gets the number of room batches that passed culling during the most recent Draw call.
         * Updated after each Draw(SpriteBatch&, OrthographicCamera&, int) call. Zero if Load has
         * not been called or Draw has not yet been called.
         */
        [[nodiscard]] int getLastVisibleRoomCountProperty() const { return lastVisibleRoomCount_; }

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
         * which is correct for pixel-art tilesets: clean, hard-edged tiles at any zoom level with
         * no edge bleeding.
         */
        [[nodiscard]] const SamplerState* getSamplerStateProperty() const { return samplerState_; }
        void setSamplerStateProperty(const SamplerState* value) { samplerState_ = value; }

        /**
         * @brief Gets/sets the sprite sort mode used for each SpriteBatch.Begin call. Defaults to
         * Deferred, which preserves draw order within a parallax group. Texture can reduce GPU
         * state changes for worlds that reference tiles from multiple tilesets, at the cost of
         * rendering order guarantees.
         */
        [[nodiscard]] SpriteSortMode getSpriteSortModeProperty() const { return spriteSortMode_; }
        void setSpriteSortModeProperty(SpriteSortMode value) { spriteSortMode_ = value; }

        /** @brief Gets/sets the custom shader effect applied when drawing tiles, or nullptr to use the default. */
        [[nodiscard]] Effect* getEffectProperty() const { return effect_; }
        void setEffectProperty(Effect* value) { effect_ = value; }

        /**
         * @brief Loads a collection of tilemaps (non-owning) for world-map rendering. All tile
         * positions are baked into world space at load time, grouped by (WorldDepth, parallax factor).
         */
        void Load(const std::vector<Tilemap*>& tilemaps);

        /** @brief Loads a TilemapWorld's levels for world-map rendering. */
        void Load(const TilemapWorld& world);

        /** @brief Unloads all world data. */
        void Unload();

        /** @brief Updates animated tiles. */
        void Update(const GameTime& gameTime);

        /** @brief Draws all tilemaps at depth 0. @throws System::InvalidOperationException Load has not been called. */
        void Draw(SpriteBatch& spriteBatch, OrthographicCamera& camera) { Draw(spriteBatch, camera, 0); }

        /**
         * @brief Draws all tilemaps at @p worldDepth. One SpriteBatch.Begin/End pair is issued
         * per unique parallax factor; all rooms sharing a parallax factor are drawn in that
         * single batch. Per-room and per-tile culling against the camera bounds is applied
         * within each batch.
         * @throws System::InvalidOperationException Load has not been called.
         */
        void Draw(SpriteBatch& spriteBatch, OrthographicCamera& camera, int worldDepth);

    private:
        struct WorldGroup;

        static void DrawTile(SpriteBatch& spriteBatch, Texture2D& texture, const Vector2& position, const Rectangle& sourceRect,
                              TilemapTileFlipFlags flipFlags, int tileWidth, int tileHeight, const Color& color);

        std::vector<std::unique_ptr<WorldGroup>> worldGroups_;
        std::vector<TilemapTileData*> animatedTiles_;
        bool isLoaded_ = false;
        int lastVisibleRoomCount_ = 0;

        BlendState blendState_ = BlendState::NonPremultiplied;
        const SamplerState* samplerState_ = &SamplerState::PointClamp;
        SpriteSortMode spriteSortMode_ = SpriteSortMode::Deferred;
        Effect* effect_ = nullptr;
    };
}
