// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::TilemapTileset3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Tilemaps::TilemapTileset, deliberately scoped down:
// maps a tile ID directly to a whole Texture2D (applied uniformly to all 6 faces of that
// tile's cube by CubeMeshRenderSystemEXT::DrawCubeEXT), not upstream's per-tile source
// rectangle within a shared texture atlas. A voxel/block world conventionally gives each
// block type its own whole texture (or texture per face, itself out of scope here) rather
// than packing many block types into one shared atlas the way a 2D tile sheet does --
// matches plan3d.md's Phase 8 "decide the exact shape... based on what a real voxel/block
// world actually needs".
#pragma once

#include <unordered_map>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Maps tile IDs to the whole-cube texture drawn on every face of that tile.
     * @see Tilemap3DEXT, whose tile IDs this tileset resolves.
     * @see TilemapRenderer3DEXT, which looks up each visible tile's texture here.
     */
    class TilemapTileset3DEXT
    {
    public:
        TilemapTileset3DEXT() = default;

        /** @brief Associates @p tileId with @p texture. Not owned by this tileset. */
        void SetTileTextureEXT(int tileId, Microsoft::Xna::Framework::Graphics::Texture2D* texture);

        /** @brief Gets the texture associated with @p tileId, or nullptr if none was set. */
        [[nodiscard]] Microsoft::Xna::Framework::Graphics::Texture2D* GetTileTextureEXT(int tileId) const;

    private:
        std::unordered_map<int, Microsoft::Xna::Framework::Graphics::Texture2D*> texturesByTileIdEXT_;
    };
}
