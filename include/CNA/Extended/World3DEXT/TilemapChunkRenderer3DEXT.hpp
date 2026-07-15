// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TilemapChunkRenderer3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design (Phase 12 E, 2026-07-15,
// user-requested "extend later" item).
//
// An OPT-IN alternative to TilemapRenderer3DEXT's per-tile draw calls, not a replacement --
// matches this project's established "keep the simple version, add an alternative for when
// you need it" pattern (e.g. Octree3DEXT alongside SpatialHash3DEXT, Phase 11 B).
// TilemapRenderer3DEXT.hpp's own header comment already documented this exact future path:
// "per-chunk batching (merging many tiles' geometry into one draw call, the way
// TilemapRenderer.cpp's own 2D layer batching does) remains a documented future
// optimization."
//
// RebuildEXT groups every visible tile FACE (naive neighbor-based culling: a face is only
// emitted if the neighboring cell along that face's normal is empty -- Tilemap3DEXT::
// HasTileEXT) into one combined VertexBuffer/IndexBuffer per (chunk coordinate, texture)
// pair -- sub-grouping by texture within each chunk mirrors 2D TilemapRenderer.cpp's own
// per-texture TileBatch precedent, since one draw call can only bind one texture and
// different tile IDs can resolve to different textures (TilemapTileset3DEXT::
// GetTileTextureEXT). Reuses CubeMeshRenderSystemEXT::BuildUnitCubeMeshEXT's exact per-face
// corner/normal/UV layout, just baking each face's vertices directly into world space
// (position = tile world center + local corner * tileSize) instead of drawing a shared unit
// mesh once per tile with a per-draw world matrix.
//
// RebuildEXT is a full rebuild, not incremental -- call it again whenever the tilemap
// changes. This matches SpatialHash3DEXT::Reset()/Octree3DEXT::Reset()/QuadTreeSpace::
// Reset()'s own established "full rebuild, not incremental" precedent (see those files'
// header comments for the same reasoning: simplicity over an incremental-update fast path
// with no demonstrated need yet).
//
// Frustum culling happens per (chunk, texture) BATCH (one BoundingBox built from its own
// baked vertices via BoundingBox::CreateFromPoints), not per tile -- coarser than
// TilemapRenderer3DEXT's per-tile culling, but that coarseness is the whole point: far
// fewer, larger draw calls is the batching win this phase exists for.
//
// ChunkBatchEXT is a private, forward-declared nested struct (full definition in the .cpp
// only) since the number of chunk batches is only known at RebuildEXT time -- an incomplete
// type inside std::vector<std::unique_ptr<T>> requires the owning class's destructor to be
// defined where T is complete (see ParticleEmitter3DEXT's Phase 11 C-1 precedent for the
// identical requirement; unlike that class, this one has no move operations to define
// alongside it -- see the constructor declarations below for why).
#pragma once

#include "Microsoft/Xna/Framework/Graphics/BasicEffect.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;
    class Tilemap3DEXT;
    class TilemapTileset3DEXT;

    /**
     * @brief Draws a Tilemap3DEXT as a small number of batched, per-chunk/per-texture draw
     * calls (only visible tile faces included), instead of TilemapRenderer3DEXT's one draw
     * call per tile.
     * @see TilemapRenderer3DEXT, the simpler per-tile alternative this doesn't replace.
     */
    class TilemapChunkRenderer3DEXT
    {
    public:
        /** @brief Default chunk extent (in tiles) along each axis; the standard voxel-engine default. */
        static constexpr int DefaultChunkSizeEXT = 16;

        /**
         * @param graphicsDevice The device chunk geometry is uploaded to and drawn through. Not owned; must outlive this renderer.
         * @param camera The camera supplying view/projection/frustum. Not owned; must outlive this renderer.
         * @param chunkSize Extent (in tiles) of one chunk along each axis. Must be positive.
         */
        explicit TilemapChunkRenderer3DEXT(Microsoft::Xna::Framework::Graphics::GraphicsDevice& graphicsDevice, Camera3DEXT& camera,
                                            int chunkSize = DefaultChunkSizeEXT);
        // No move/copy: BasicEffect (held by value, matching CubeMeshRenderSystemEXT's own
        // precedent) has a private copy constructor and no move operations of its own, so
        // this type is constructed once and never relocated -- matching every other
        // renderer-shaped type in World3DEXT (CubeMeshRenderSystemEXT, DebugDrawSystemEXT,
        // TextBillboardRenderSystemEXT), none of which are movable either. The destructor
        // still needs an explicit out-of-line definition (in the .cpp, where ChunkBatchEXT
        // is a complete type) purely for the incomplete-type-in-vector<unique_ptr<T>>
        // requirement -- see this file's header comment.
        ~TilemapChunkRenderer3DEXT();
        TilemapChunkRenderer3DEXT(const TilemapChunkRenderer3DEXT&) = delete;
        TilemapChunkRenderer3DEXT& operator=(const TilemapChunkRenderer3DEXT&) = delete;

        /**
         * @brief Rebuilds every chunk batch from @p tilemap/@p tileset's current state.
         * A full rebuild, not incremental -- call again whenever the tilemap changes.
         */
        void RebuildEXT(const Tilemap3DEXT& tilemap, const TilemapTileset3DEXT& tileset);

        /** @brief Gets the number of (chunk, texture) batches RebuildEXT last produced (for tests/diagnostics). */
        [[nodiscard]] int GetChunkBatchCountEXT() const;

        /** @brief Gets the total triangle count across every batch RebuildEXT last produced -- lower than 2 * 6 * (populated tile count) whenever neighbor-based face culling hid shared internal faces (for tests/diagnostics). */
        [[nodiscard]] int GetTotalPrimitiveCountEXT() const;

        /** @brief Draws every chunk batch whose bounds intersect the camera's current frustum. */
        void Draw();

    private:
        struct ChunkBatchEXT;

        Microsoft::Xna::Framework::Graphics::GraphicsDevice* graphicsDeviceEXT_;
        Camera3DEXT* cameraEXT_;
        int chunkSizeEXT_;
        Microsoft::Xna::Framework::Graphics::BasicEffect effectEXT_;
        std::vector<std::unique_ptr<ChunkBatchEXT>> chunksEXT_;
    };
}
