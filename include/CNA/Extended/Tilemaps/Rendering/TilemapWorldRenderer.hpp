// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapWorldRenderer.cs.
//
// *** ARCHITECTURAL DEVIATION: DefaultEffect -> BasicEffect (not a literal port) ***
// See TilemapRenderer.hpp's top comment for the full rationale (genuine XNA `BasicEffect` vs.
// MonoGame.Extended's own `DefaultEffect`, CNA's `Effect` base having no generic
// `Parameters`/`CurrentTechnique.Passes` reflection surface, and the idiomatic
// SetVertexBuffer/SetIndexBuffer/Apply()/DrawIndexedPrimitives flow this port uses instead) -- it
// applies identically here. The public `Effect Effect { get; set; }` property is likewise narrowed
// to `BasicEffect&` for the same reason.
//
// `Dictionary<WorldGroupKey, WorldGroup>`-shaped grouping does not exist in this class the way it
// does in `TilemapWorldSpriteBatchRenderer.cs` -- upstream `TilemapWorldRenderer.cs` groups
// directly into a local `Dictionary<TileBatchKey, TileAccumulator>` inside `Load`, where
// `TileBatchKey` is a `readonly record struct(int WorldDepth, Texture2D Texture, Vector2
// ParallaxFactor)`. Matching `TilemapWorldSpriteBatchRenderer.hpp`'s own documented precedent for
// its structurally identical `WorldGroupKey`/`WorldRoomKey` local grouping problem: a
// `Vector2`-containing key has no `std::hash` specialization in this project, and load-time group
// counts are small (bounded by distinct (depth, texture, parallax) combinations, not tile count),
// so this port uses a linear scan over a `std::vector<TileAccumulator>` instead of a hash map --
// entirely local to `Load()`'s .cpp implementation, never exposed here.
//
// `List<WorldBatch> _worldBatches` -> `std::vector<WorldBatch> worldBatches_`, a private nested
// struct kept COMPLETE in this header (or the mapped_type of this class's own member field is
// under-defined) -- unlike `TilemapWorldSpriteBatchRenderer.hpp`'s forward-declared `WorldGroup`
// (whose owning container is `std::vector<std::unique_ptr<WorldGroup>>`, needing only the pointee
// forward-declared), `worldBatches_` here holds `WorldBatch` directly by value, so it must be
// complete wherever this class definition is parsed.
//
// `IEnumerable<Tilemap> tilemaps` -> `const std::vector<Tilemap*>&` (non-owning: this renderer
// never owns the tilemaps it bakes), matching `TilemapWorldSpriteBatchRenderer::Load`'s identical
// translation of the same upstream parameter shape.
#pragma once

#include "CNA/Extended/OrthographicCamera.hpp"
#include "CNA/Extended/Tilemaps/Rendering/LayerModel.hpp"
#include "CNA/Extended/Tilemaps/TilemapWorld.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "System/IDisposable.hpp"

#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Graphics::BasicEffect;
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::SamplerState;

    using CNA::Extended::Tilemaps::Tilemap;
    using CNA::Extended::Tilemaps::TilemapWorld;

    /**
     * @brief High-performance world-map renderer using GraphicsDevice directly.
     *
     * Pre-bakes all tile geometry from a collection of tilemaps into world-space vertex buffers
     * grouped by (WorldDepth, texture, parallax factor). Each group produces a single draw call
     * regardless of how many rooms contributed tiles to it.
     *
     * Use Draw(OrthographicCamera&, int) to render a specific depth layer, or
     * Draw(OrthographicCamera&) to render depth 0.
     *
     * Animated tiles are not currently supported in world mode: tile geometry is baked at load
     * time and is not rebuilt per-frame. Use TilemapWorldSpriteBatchRenderer if animated tile
     * support is required.
     *
     * See TilemapRenderer.hpp's own top comment for the DefaultEffect -> BasicEffect substitution
     * this port makes and why (applies identically here).
     */
    class TilemapWorldRenderer final : public System::IDisposable
    {
    public:
        /** @brief Creates a renderer that draws using @p graphicsDevice (non-owning; must outlive this renderer). */
        explicit TilemapWorldRenderer(GraphicsDevice& graphicsDevice);

        ~TilemapWorldRenderer() override;
        TilemapWorldRenderer(const TilemapWorldRenderer&) = delete;
        TilemapWorldRenderer& operator=(const TilemapWorldRenderer&) = delete;
        TilemapWorldRenderer(TilemapWorldRenderer&&) = delete;
        TilemapWorldRenderer& operator=(TilemapWorldRenderer&&) = delete;

        /**
         * @brief Gets/sets the blend state used when drawing tiles. Defaults to NonPremultiplied,
         * which is correct for tileset textures loaded directly from file where the PNG pixel
         * data has not been modified. Set to AlphaBlend if your tileset textures are loaded
         * through a content pipeline that premultiplies alpha by default.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        [[nodiscard]] const BlendState& getBlendStateProperty() const;
        void setBlendStateProperty(const BlendState& value);

        /**
         * @brief Gets/sets the sampler state used when drawing tiles. Defaults to PointClamp,
         * which is correct for pixel-art tilesets: clean, hard-edged tiles at any zoom level with
         * no edge bleeding.
         * @throws System::ArgumentNullException @p value is nullptr.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        [[nodiscard]] const SamplerState* getSamplerStateProperty() const;
        void setSamplerStateProperty(const SamplerState* value);

        /**
         * @brief Gets/sets the shader effect used when drawing tiles. Defaults to an internally
         * owned BasicEffect configured for textured, vertex-colored, unlit rendering. See this
         * header's own top comment for why this is narrowed from upstream's generic `Effect`
         * property to `BasicEffect&` specifically.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        [[nodiscard]] BasicEffect& getEffectProperty() const;
        void setEffectProperty(BasicEffect& value);

        /**
         * @brief Loads a collection of tilemaps (non-owning; each must outlive this call, but not
         * necessarily this renderer -- all tile geometry is baked into world-space vertex buffers
         * during this call) as a seamless world. Each tilemap's WorldPosition/WorldDepth
         * properties determine where it is placed and which depth layer it belongs to.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void Load(const std::vector<Tilemap*>& tilemaps);

        /** @brief Loads a TilemapWorld's levels as a seamless world. See the vector overload for details. */
        void Load(const TilemapWorld& world);

        /** @brief Unloads all world data and releases GPU resources. @throws System::ObjectDisposedException this renderer has been disposed. */
        void Unload();

        /**
         * @brief Draws all tilemaps at depth 0.
         * @throws System::InvalidOperationException Load has not been called.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void Draw(OrthographicCamera& camera);

        /**
         * @brief Draws all tilemaps at @p worldDepth. Only tilemaps with this exact depth are drawn.
         * @throws System::InvalidOperationException Load has not been called.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void Draw(OrthographicCamera& camera, int worldDepth);

        /** @brief Releases all GPU resources used by this renderer. Idempotent. */
        void Dispose() override;

    private:
        /** @brief A merged draw-call batch for one (WorldDepth, texture, parallax factor) group. Ported from TilemapWorldRenderer.cs's private nested `WorldBatch`. */
        struct WorldBatch
        {
            LayerModel Model;
            int WorldDepth;
        };

        void DrawModel(const LayerModel& model);
        void ThrowIfDisposed() const;

        GraphicsDevice& graphicsDevice_;
        BasicEffect defaultEffect_;
        BasicEffect* effect_;
        BlendState blendState_ = BlendState::NonPremultiplied;
        const SamplerState* samplerState_ = &SamplerState::PointClamp;
        bool isDisposed_ = false;

        std::vector<WorldBatch> worldBatches_;
        bool isLoaded_ = false;
    };
}
