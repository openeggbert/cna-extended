// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Rendering/TilemapRenderer.cs.
//
// *** ARCHITECTURAL DEVIATION: DefaultEffect -> BasicEffect (not a literal port) ***
// Upstream constructs its own `MonoGame.Extended.Graphics.Effects.DefaultEffect` (`_defaultEffect
// = new DefaultEffect(_graphicsDevice) { TextureEnabled = true, VertexColorEnabled = true };`) and
// draws through XNA's dynamic-reflection `Effect` API: `_effect.Parameters["Texture"].SetValue(...)`
// then `foreach (EffectPass pass in _effect.CurrentTechnique.Passes) { pass.Apply(); ... }`.
//
// This project's own `DefaultEffect` (CNA::Extended::Graphics::Effects::DefaultEffect) was
// deliberately hand-authored during Phase 5 with a GLSL vertex layout matching SpriteBatch's own
// internal compact vertex format (`vec2 aPos`, `vec2 aTexCoord`, `vec4 aColor`) -- see that class's
// own header comment. That layout is NOT compatible with XNA's standard `VertexPositionColorTexture`
// (`Vector3 Position`, `Color`, `Vector2 TextureCoordinate`) that `TilemapRendererShared::
// CreateLayerModel`/`AddTileQuad` (already ported, Phase 7) build vertex/index buffers for. Rather
// than fork/extend `DefaultEffect` to add a second, incompatible vertex layout to a class that
// already documents its SpriteBatch-only scoping as a known limitation, this port uses `cna`'s real
// `Microsoft::Xna::Framework::Graphics::BasicEffect` instead:
//  - It is the genuine, standard XNA 4.0 effect class (`DefaultEffect` is MonoGame.Extended's own
//    invention, with no XNA equivalent).
//  - It is already fully implemented and pixel-verified across all 3 CNA backends (EasyGL/Vulkan/
//    Bgfx), specifically including `TextureEnabled=true` + `VertexColorEnabled=true` +
//    `LightingEnabled=false` over the exact `VertexPositionColorTexture` stride-24 vertex layout
//    this renderer uses -- see `cna/docs/basiceffect-support.md`.
//  - Its API (`World`/`View`/`Projection` public fields, `getTextureProperty()`/
//    `setTextureProperty()`, `getTextureEnabledProperty()`/`setTextureEnabledProperty()`, public
//    `VertexColorEnabled` field, `getLightingEnabledProperty()` defaulting to `false`) is a strict
//    superset of what `DefaultEffect` provided (`DefaultEffect` never had lighting at all).
// The internal `defaultEffect_` member is configured with `TextureEnabled=true`,
// `VertexColorEnabled=true` (leaving `LightingEnabled` at its default `false`) to reproduce
// `DefaultEffect`'s always-on texture+vertex-color, no-lighting behavior exactly.
//
// A second, related deviation: upstream's public `Effect Effect { get; set; }` property accepts
// ANY XNA `Effect` implementing `IEffectMatrices`, because the generic `Parameters["Texture"]`
// indexer works against arbitrary user-defined effects via XNA's runtime parameter reflection.
// CNA's `Effect` base class (see Effect.hpp's own doc comments) has NO such generic
// Parameters/Techniques collection for built-in effects like `BasicEffect` -- there is nothing to
// reflect into for a general "any effect" property. Since this renderer must set a concrete
// texture on whatever effect is active, the public `Effect` property here is narrowed to
// `BasicEffect&` specifically (still swappable -- a caller can substitute their own `BasicEffect`
// instance, e.g. to install a different `RasterizerState`-adjacent configuration -- just not an
// arbitrary unrelated `Effect` subclass). This mirrors `DefaultEffect.cpp`'s own precedent of
// re-authoring the underlying apply mechanism in idiomatic C++ rather than translating a C#
// mechanism (reflection-based `Parameters`/`CurrentTechnique.Passes`) that has no CNA equivalent.
//
// Draw mechanics: instead of the `Parameters["Texture"].SetValue(...)` / `foreach (EffectPass pass
// in effect.CurrentTechnique.Passes) { pass.Apply(); ... }` walk, this port uses CNA's real, idiomatic
// draw flow (see Effect.hpp/GraphicsDevice.hpp doc comments and BasicEffectTests.cpp /
// rasterizerstate_cullmode_indexed_basiceffect_test.cpp for the established pattern this mirrors):
// set effect properties (Texture, View/Projection/World) -> `GraphicsDevice::SetVertexBuffer` /
// `SetIndexBuffer` -> `effect.Apply()` (which internally calls `GraphicsDevice::SetCurrentEffect`
// and the effect's own `OnApply()`) -> `GraphicsDevice::DrawIndexedPrimitives(...)`. The backend
// picks up the effect's state via `FillGpuDrawParams()` automatically; there is no
// technique/pass collection to walk.
//
// `IDisposable`/`ThrowIfDisposed()` -> `System::IDisposable` + `System::ObjectDisposedException::
// ThrowIf`, matching this project's established pattern for GPU-resource-owning types that need
// deterministic release (see `CNA::Extended::Particles::ParticleEmitter`).
//
// `Dictionary<TilemapLayer, string>` (layer -> group name) -> `System::Collections::Generic::
// Dictionary<TilemapLayer*, std::string>` (both key and value are copyable, matching this
// project's established Dictionary<K,V> convention). `Dictionary<string, LayerGroup>`/
// `Dictionary<TilemapTileLayer, List<LayerModel>>`/`Dictionary<TilemapImageLayer, LayerModel>`/
// `Dictionary<TilemapImageLayer, RepeatImageLayerModel>`/`Dictionary<TilemapObjectLayer,
// List<LayerModel>>` all have a move-only mapped type (`LayerGroup`/`LayerModel`/
// `RepeatImageLayerModel` all own GPU `VertexBuffer`/`IndexBuffer` resources), which `System::
// Collections::Generic::Dictionary`'s copy-based `TryGetValue`/indexer API cannot store -- plain
// `std::unordered_map<K*, V>` is used directly for these instead, matching the identical, already-
// established precedent in `TilemapTileset.hpp` (`std::unordered_map<int,
// std::unique_ptr<TilemapTileData>> tileData_`, documented there in full) and
// `TilemapWorldSpriteBatchRenderer.cpp`. `HashSet<string>` (`_drawnGroupsBuffer`/
// `_groupsWithAnimations`) -> plain `std::unordered_set<std::string>`, matching
// `TilemapWorldSpriteBatchRenderer.cpp`'s identical `HashSet<TilemapTileData>` ->
// `std::unordered_set<const TilemapTileData*>` precedent (no custom hash needed for `std::string`
// either).
//
// `LayerGroup`/`RepeatImageLayerModel` are private nested types (matching upstream's own private
// nested classes) kept COMPLETE in this header (not just forward-declared, unlike
// `TilemapWorldSpriteBatchRenderer.hpp`'s `WorldGroup`) because they are the mapped_type of member
// fields (`layerGroups_`/`repeatImageLayerModels_`) declared in this class, which requires them
// complete at this point -- same reasoning as `TilemapSpriteBatchRenderer.hpp`'s `TileRenderPadding`.
// `RepeatImageLayerModel`'s fields are named in lowerCamelCase (`vertexBuffer`/`indexBuffer`/...)
// rather than upstream's PascalCase (`VertexBuffer`/`IndexBuffer`/...): a field named `IndexBuffer`
// of type `Microsoft::Xna::Framework::Graphics::IndexBuffer` (and similarly `Color` of type
// `Color`) would shadow the type name for the remainder of the class body, a needless landmine for
// a private, non-property-exposed struct with no C# get/set surface to mirror.
//
// The private nested `record TileBatch`/`readonly struct TileBatchKey`-shaped helper used inside
// `BuildLayerModels(TilemapTileLayer&)`/`RebuildLayerGroupInternal`/`BuildMergedUngroupedModels` is
// declared entirely inside the .cpp (not this header), since it is a local implementation detail of
// those method bodies, never a member field's type -- matching
// `TilemapWorldSpriteBatchRenderer.cpp`'s identical treatment of its own local grouping keys.
//
// `Tilemap tilemap` parameter of `LoadTilemap` -> `Tilemap*` (nullable, preserving upstream's
// `ArgumentNullException.ThrowIfNull(tilemap)` check) rather than a dropped-null-check reference,
// because `tilemap_` itself must be a nullable field (starts null, cleared by `UnloadTilemap()`) --
// matching `TilemapSpriteBatchRenderer::LoadTilemap(Tilemap*)`'s identical precedent.
// `OrthographicCamera camera` parameters elsewhere (`BeginDraw`/`Draw`/`DrawLayers`) -> `
// OrthographicCamera&` with the null check dropped, since those are immediate-use non-null
// parameters with no such nullable-field justification (matching `TilemapSpriteBatchRenderer`'s
// identical `OrthographicCamera&` convention). `params string[] layerNames` -> `const
// std::vector<std::string>&`, matching `TilemapSpriteBatchRenderer::DrawLayers`'s established
// translation of the same C# construct.
//
// `LayerModel model` nullable parameter of `DrawLayerModel` -> `const LayerModel&` with the null
// check dropped: every real call site passes an element from a `std::vector<LayerModel>`/
// `std::unordered_map<K, LayerModel>` entry, never a null placeholder (C++ containers of `LayerModel`
// by value cannot hold "null" elements the way a C# `List<LayerModel>` of nullable reference-type
// elements formally could), so the defensive `model == null` guard has no reachable
// C++ equivalent case.
//
// `model.Dispose()` walks in `UnloadTilemap()`/`RemoveLayerGroup()`/`RebuildLayerGroupInternal()` /
// `BuildMergedUngroupedModels()` are NOT translated: `LayerModel`/`RepeatImageLayerModel` are plain
// RAII types in this port (their owned `VertexBuffer`/`IndexBuffer` release their GPU handles in
// their own destructors -- see `LayerModel.hpp`'s header comment), so clearing/erasing the
// container that owns them is sufficient; there is nothing left to dispose manually.
//
// Upstream's real behavior (confirmed by reading every `Color` construction site in
// `TilemapRenderer.cs`): only per-layer `Opacity` is baked into the vertex color alpha for tile,
// image, and object layers -- `TilemapLayer.TintColor` is read nowhere in this class, unlike its
// SpriteBatch-based sibling `TilemapSpriteBatchRenderer`, which does honor `TintColor`. This is
// ported as-is (a genuine upstream inconsistency between the two renderers, not a bug this port
// should silently "fix").
#pragma once

#include "CNA/Extended/OrthographicCamera.hpp"
#include "CNA/Extended/Tilemaps/Rendering/LayerModel.hpp"
#include "CNA/Extended/Tilemaps/Rendering/RenderMode.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"
#include "Microsoft/Xna/Framework/Graphics/BlendState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DepthStencilState.hpp"
#include "Microsoft/Xna/Framework/Graphics/DynamicVertexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/IndexBuffer.hpp"
#include "Microsoft/Xna/Framework/Graphics/RasterizerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/VertexPositionColorTexture.hpp"
#include "System/Collections/Generic/Dictionary.hpp"
#include "System/IDisposable.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
    class Texture2D;
}

namespace CNA::Extended::Tilemaps
{
    class Tilemap;
    class TilemapLayer;
    class TilemapTileLayer;
    class TilemapImageLayer;
    class TilemapObjectLayer;
    class TilemapTileData;
}

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::BasicEffect;
    using Microsoft::Xna::Framework::Graphics::BlendState;
    using Microsoft::Xna::Framework::Graphics::DepthStencilState;
    using Microsoft::Xna::Framework::Graphics::DynamicVertexBuffer;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::IndexBuffer;
    using Microsoft::Xna::Framework::Graphics::RasterizerState;
    using Microsoft::Xna::Framework::Graphics::SamplerState;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    using CNA::Extended::Tilemaps::Tilemap;
    using CNA::Extended::Tilemaps::TilemapImageLayer;
    using CNA::Extended::Tilemaps::TilemapLayer;
    using CNA::Extended::Tilemaps::TilemapObjectLayer;
    using CNA::Extended::Tilemaps::TilemapTileData;
    using CNA::Extended::Tilemaps::TilemapTileLayer;

    /**
     * @brief High-performance tilemap renderer using GraphicsDevice directly.
     *
     * Renders tiles using vertex and index buffers. Supports layer grouping to merge multiple
     * layers into a single draw call, significantly reducing draw call overhead for maps with
     * many layers. Groups can be defined dynamically and updated at runtime; they rebuild
     * automatically when modified.
     *
     * See this header's own top comment for the DefaultEffect -> BasicEffect substitution this
     * port makes and why.
     */
    class TilemapRenderer final : public System::IDisposable
    {
    public:
        /** @brief Creates a renderer that draws using @p graphicsDevice (non-owning; must outlive this renderer). */
        explicit TilemapRenderer(GraphicsDevice& graphicsDevice);

        ~TilemapRenderer() override;
        TilemapRenderer(const TilemapRenderer&) = delete;
        TilemapRenderer& operator=(const TilemapRenderer&) = delete;
        TilemapRenderer(TilemapRenderer&&) = delete;
        TilemapRenderer& operator=(TilemapRenderer&&) = delete;

        /** @brief Gets the current default rendering mode. @throws System::ObjectDisposedException this renderer has been disposed. */
        [[nodiscard]] RenderMode getDefaultRenderModeProperty() const;

        /**
         * @brief Gets/sets the blend state used when drawing tiles. Defaults to NonPremultiplied,
         * which is correct for tileset textures loaded directly from file (e.g. via a stream or
         * custom loader) where the PNG pixel data has not been modified. Set to AlphaBlend if
         * your tileset textures are loaded through a content pipeline that premultiplies alpha by
         * default -- note that layer opacity will not apply correctly to the RGB channel in that
         * mode, because the renderer bakes opacity only into the vertex color alpha at load time.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        [[nodiscard]] const BlendState& getBlendStateProperty() const;
        void setBlendStateProperty(const BlendState& value);

        /**
         * @brief Gets/sets the sampler state used when drawing tiles. Defaults to PointClamp,
         * which is correct for pixel-art tilesets: clean, hard-edged tiles at any zoom level or
         * rotation angle with no edge bleeding between adjacent tiles in the texture atlas.
         * LinearClamp enables smooth sub-pixel interpolation for non-pixel-art tilesets (keep the
         * camera at integer world-pixel coordinates to avoid edge-texel bleed; not recommended
         * with camera rotation). For repeating image layers, the renderer automatically uses the
         * wrap equivalent of the configured state (PointClamp -> PointWrap, LinearClamp ->
         * LinearWrap; any other state, including an already-wrapping one, is used as-is).
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

        /** @brief Gets the names of all defined layer groups. @throws System::ObjectDisposedException this renderer has been disposed. */
        [[nodiscard]] std::vector<std::string> getLayerGroupsProperty() const;

        /**
         * @brief Loads a tilemap (non-owning; must outlive this renderer or a subsequent
         * LoadTilemap/UnloadTilemap call) for rendering.
         * @throws System::ArgumentNullException @p tilemap is nullptr.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void LoadTilemap(Tilemap* tilemap);

        /** @brief Unloads the current tilemap and releases all GPU resources. @throws System::ObjectDisposedException this renderer has been disposed. */
        void UnloadTilemap();

        /**
         * @brief Sets the default rendering mode for ungrouped layers. Affects Draw(OrthographicCamera&).
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void SetDefaultRenderMode(RenderMode mode);

        /**
         * @brief Defines a layer group for merged rendering: all layers in the group are drawn in
         * a single draw call, in the order specified (back to front). Groups can be redefined at
         * any time; the merged buffers rebuild when the group is next drawn. Each layer can only
         * belong to one group -- if a layer is already in another group, it is moved.
         * @throws System::Collections::Generic::KeyNotFoundException any layer name is not found in the tilemap.
         * @throws System::InvalidOperationException no tilemap is loaded.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DefineLayerGroup(const std::string& groupName, const std::vector<std::string>& layerNames);

        /**
         * @brief Defines a layer group using a contiguous run of layer indices. See the by-name
         * overload for grouping semantics.
         * @throws System::ArgumentOutOfRangeException @p startIndex or @p count is out of range.
         * @throws System::InvalidOperationException no tilemap is loaded.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DefineLayerGroup(const std::string& groupName, int startIndex, int count);

        /**
         * @brief Removes a layer group. Its layers become ungrouped and are drawn individually by
         * a subsequent Draw(OrthographicCamera&) call. No-op if the group does not exist.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void RemoveLayerGroup(const std::string& groupName);

        /** @brief Gets whether a layer group is defined. @throws System::ObjectDisposedException this renderer has been disposed. */
        [[nodiscard]] bool HasLayerGroup(const std::string& groupName) const;

        /**
         * @brief Marks a layer group as needing rebuild. Call after modifying tiles in layers
         * that belong to a group; the group rebuilds on its next draw.
         * @throws System::ArgumentException the group does not exist.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void MarkGroupDirty(const std::string& groupName);

        /**
         * @brief Rebuilds the merged vertex/index buffers for a layer group immediately, rather
         * than lazily on next draw. Useful to control when the rebuild cost happens (e.g. during
         * loading).
         * @throws System::ArgumentException the group does not exist.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void RebuildLayerGroup(const std::string& groupName);

        /**
         * @brief Begins a manual rendering sequence: call before DrawLayerGroup/DrawLayer(string)/
         * DrawLayer(int). Must be paired with EndDraw(). Automatically saves GraphicsDevice state
         * for restoration after interleaved SpriteBatch usage.
         * @throws System::InvalidOperationException no tilemap is loaded, or BeginDraw was already called.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void BeginDraw(OrthographicCamera& camera);

        /**
         * @brief Draws a layer group: all its layers render in a single draw call. Must be called
         * between BeginDraw/EndDraw. Rebuilds the group first if it is marked dirty.
         * @throws System::ArgumentException the group does not exist.
         * @throws System::InvalidOperationException called outside of BeginDraw/EndDraw.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DrawLayerGroup(const std::string& groupName);

        /**
         * @brief Draws a single layer by name. If the layer belongs to a group, only this layer
         * is drawn (not the entire group). Must be called between BeginDraw/EndDraw.
         * @throws System::Collections::Generic::KeyNotFoundException the layer does not exist.
         * @throws System::InvalidOperationException called outside of BeginDraw/EndDraw.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DrawLayer(const std::string& layerName);

        /**
         * @brief Draws a single layer by index. See the by-name overload for grouping semantics.
         * @throws System::ArgumentOutOfRangeException @p layerIndex is out of range.
         * @throws System::InvalidOperationException called outside of BeginDraw/EndDraw.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DrawLayer(int layerIndex);

        /**
         * @brief Convenience wrapper around BeginDraw/DrawLayer(string)/EndDraw: draws a subset of
         * layers by name, in the order specified. Group merging is bypassed -- each layer is
         * drawn individually. For interleaving tilemap layers with sprite/entity drawing, use the
         * manual BeginDraw/DrawLayer/EndDraw path instead.
         * @throws System::Collections::Generic::KeyNotFoundException any layer name is not found.
         * @throws System::InvalidOperationException no tilemap is loaded.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void DrawLayers(OrthographicCamera& camera, const std::vector<std::string>& layerNames);

        /**
         * @brief Ends a manual rendering sequence started by BeginDraw().
         * @throws System::InvalidOperationException BeginDraw was not called.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void EndDraw();

        /**
         * @brief Draws all visible layers automatically. Grouped layers draw as a group (single
         * draw call); ungrouped layers draw according to DefaultRenderMode. For fine-grained
         * control over layer order (e.g. to inject sprites between layers), use the manual
         * BeginDraw/DrawLayerGroup/DrawLayer/EndDraw path instead.
         * @throws System::InvalidOperationException no tilemap is loaded.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void Draw(OrthographicCamera& camera);

        /**
         * @brief Updates animated tiles. Must be called each frame to advance tile animations;
         * automatically marks groups containing animated tiles as dirty when a frame changes.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void Update(const GameTime& gameTime);

        /**
         * @brief Saves the current GraphicsDevice state. Call before using SpriteBatch between
         * BeginDraw/EndDraw, paired with RestoreGraphicsDeviceState(). BeginDraw() already saves
         * state automatically, so this is only needed for additional state changes.
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void SaveGraphicsDeviceState();

        /**
         * @brief Restores previously saved GraphicsDevice state. Call after using SpriteBatch
         * between BeginDraw/EndDraw, paired with SaveGraphicsDeviceState().
         * @throws System::ObjectDisposedException this renderer has been disposed.
         */
        void RestoreGraphicsDeviceState();

        /** @brief Releases all GPU resources used by this renderer. Idempotent. */
        void Dispose() override;

    private:
        /** @brief A named collection of tile layers merged into one draw call. Ported from TilemapRenderer.cs's private nested `LayerGroup`. */
        struct LayerGroup
        {
            std::vector<TilemapLayer*> Layers;
            std::vector<LayerModel> MergedModels;
            bool IsDirty = true;
        };

        /**
         * @brief GPU state for a repeating (RepeatX/RepeatY) image layer: a 4-vertex quad rebuilt
         * every draw from the camera's visible world region. Ported from TilemapRenderer.cs's
         * private nested `RepeatImageLayerModel`. See this header's own top comment for the
         * lowerCamelCase field naming (avoids a field/type name collision upstream's PascalCase
         * property names would cause here).
         */
        struct RepeatImageLayerModel
        {
            RepeatImageLayerModel(DynamicVertexBuffer vb, IndexBuffer ib, Texture2D* tex, bool repeatXValue, bool repeatYValue,
                                   const Vector2& layerPositionValue, const Microsoft::Xna::Framework::Color& colorValue)
                : vertexBuffer(std::move(vb)), indexBuffer(std::move(ib)), texture(tex), repeatX(repeatXValue), repeatY(repeatYValue),
                  layerPosition(layerPositionValue), color(colorValue)
            {
            }

            /** @brief Recomputes the 4 vertex positions/UVs from the camera's visible world region and uploads them. */
            void UpdateVertices(const Matrix& inverseViewProjection, const Vector2& parallaxOffset);

            DynamicVertexBuffer vertexBuffer;
            IndexBuffer indexBuffer;
            Texture2D* texture;
            bool repeatX;
            bool repeatY;
            Vector2 layerPosition;
            Microsoft::Xna::Framework::Color color;
        };

        void BuildLayerModels();
        [[nodiscard]] std::vector<LayerModel> BuildLayerModels(TilemapTileLayer& tileLayer);
        [[nodiscard]] std::optional<LayerModel> BuildImageLayerModel(TilemapImageLayer& imageLayer);
        [[nodiscard]] std::optional<RepeatImageLayerModel> BuildRepeatImageLayerModel(TilemapImageLayer& imageLayer);
        [[nodiscard]] std::vector<LayerModel> BuildObjectLayerModels(TilemapObjectLayer& objectLayer);
        static void AddObjectTileQuad(std::vector<Microsoft::Xna::Framework::Graphics::VertexPositionColorTexture>& vertices,
                                       std::vector<int>& indices, const Vector2& pivot, float width, float height, float rotation,
                                       const Microsoft::Xna::Framework::Rectangle& sourceRect, TilemapTileFlipFlags flipFlags,
                                       const Texture2D& texture, const Microsoft::Xna::Framework::Color& color);

        void ApplyParallaxWorld(const Vector2& parallaxFactor);
        void DrawLayerModel(const LayerModel& model);
        void DrawRepeatImageLayerModel(RepeatImageLayerModel& model);
        void DrawLayerInternal(TilemapLayer& layer);
        void RebuildLayerGroupInternal(LayerGroup& group);
        void BuildMergedUngroupedModels();
        void BuildAnimatedTilesList();
        void UpdateAnimationSets();

        void ThrowIfDisposed() const;
        void ThrowIfNoTilemap() const;
        void ThrowIfNotDrawing() const;

        GraphicsDevice& graphicsDevice_;
        BasicEffect defaultEffect_;
        BasicEffect* effect_;
        Tilemap* tilemap_ = nullptr;
        RenderMode defaultRenderMode_ = RenderMode::Merged;
        BlendState blendState_ = BlendState::NonPremultiplied;
        const SamplerState* samplerState_ = &SamplerState::PointClamp;
        bool isDisposed_ = false;

        BlendState savedBlendState_;
        SamplerState savedSamplerState_;
        RasterizerState savedRasterizerState_;
        DepthStencilState savedDepthStencilState_;

        std::unordered_map<std::string, LayerGroup> layerGroups_;
        System::Collections::Generic::Dictionary<TilemapLayer*, std::string> layerToGroup_;
        std::unordered_map<TilemapTileLayer*, std::vector<LayerModel>> layerModels_;
        std::unordered_map<TilemapImageLayer*, LayerModel> imageLayerModels_;
        std::unordered_map<TilemapImageLayer*, RepeatImageLayerModel> repeatImageLayerModels_;
        std::unordered_map<TilemapObjectLayer*, std::vector<LayerModel>> objectLayerModels_;
        std::vector<LayerModel> mergedUngroupedModels_;
        bool mergedUngroupedDirty_ = false;
        std::unordered_set<std::string> drawnGroupsBuffer_;

        std::vector<TilemapTileData*> animatedTiles_;
        std::unordered_set<std::string> groupsWithAnimations_;
        bool mergedUngroupedHasAnimations_ = false;

        bool isDrawing_ = false;
        OrthographicCamera* camera_ = nullptr;
    };
}
