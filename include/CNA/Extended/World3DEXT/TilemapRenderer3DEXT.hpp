// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TilemapRenderer3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Tilemaps::Rendering::TilemapRenderer: a standalone
// renderer object with its own Draw(camera, ...) method, matching the real 2D
// TilemapRenderer's own shape (a renderer instantiated and driven directly by game code,
// not an ECS system -- unlike every *System3DEXT type elsewhere in World3DEXT, this one
// deliberately mirrors upstream's actual non-ECS shape, since plan3d.md's own Phase 8
// bullet names it "TilemapRenderer3DEXT", not "...System3DEXT").
//
// Batches visible tiles into CubeMeshRenderSystemEXT::DrawCubeEXT calls (Phase 6's shared
// cube mesh/effect, extracted for exactly this kind of reuse), frustum-culling each tile
// individually via Camera3DEXT's BoundingFrustum -- the simplest correct version of the
// "per-chunk/per-tile culling" plan3d.md calls for; per-chunk batching (merging many tiles'
// geometry into one draw call, the way TilemapRenderer.cpp's own 2D layer batching does)
// remains a documented future optimization if profiling ever shows individual per-tile
// draw calls are a real bottleneck.
#pragma once

namespace CNA::Extended::World3DEXT
{
    class Camera3DEXT;
    class CubeMeshRenderSystemEXT;
    class Tilemap3DEXT;
    class TilemapTileset3DEXT;

    /**
     * @brief Draws every non-empty tile in a Tilemap3DEXT as a textured cube, via
     * CubeMeshRenderSystemEXT::DrawCubeEXT, frustum-culling tiles outside the camera's view.
     * @see Tilemap3DEXT, the tile grid this renderer draws.
     * @see TilemapTileset3DEXT, resolving each tile ID to a texture.
     */
    class TilemapRenderer3DEXT
    {
    public:
        /** @param cubeRenderSystem The system this renderer draws every tile through. Not owned; must outlive this renderer. */
        explicit TilemapRenderer3DEXT(CubeMeshRenderSystemEXT& cubeRenderSystem);

        /**
         * @brief Draws every non-empty tile in @p tilemap that intersects @p camera's current frustum.
         * @param camera Supplies the frustum tiles are culled against and the view/projection used to draw them.
         * @param tilemap The tile grid to draw.
         * @param tileset Resolves each tile ID to a texture.
         */
        void Draw(Camera3DEXT& camera, const Tilemap3DEXT& tilemap, const TilemapTileset3DEXT& tileset);

    private:
        CubeMeshRenderSystemEXT* cubeRenderSystemEXT_;
    };
}
