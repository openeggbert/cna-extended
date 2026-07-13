// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTile.cs. `GetLocalId(tilesets, out tileset)`'s
// nullable `out TilemapTileset tileset` (null represents the empty-tile sentinel) is translated
// as a `TilemapTileset*&` (reference-to-pointer) out-parameter, since `TilemapTileset` instances
// are owned by `TilemapTilesetCollection`, not by this call.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"

namespace CNA::Extended::Tilemaps
{
    class TilemapTilesetCollection;
    class TilemapTileset;
    class TilemapTileData;

    /** @brief A tile instance in a tile layer: a global tile ID (identifying both tileset and local tile) plus flip flags. */
    class TilemapTile
    {
    public:
        TilemapTile() = default;

        /** @brief Creates a tile referencing @p globalId, with optional @p flipFlags (default None). */
        explicit TilemapTile(int globalId, TilemapTileFlipFlags flipFlags = TilemapTileFlipFlags::None);

        /** @brief Gets the global tile ID (combines the owning tileset's first global ID with the local tile ID). */
        [[nodiscard]] int getGlobalIdProperty() const { return globalId_; }

        /** @brief Gets the flip transformation flags applied to this tile. */
        [[nodiscard]] TilemapTileFlipFlags getFlipFlagsProperty() const { return flipFlags_; }

        /** @brief Gets the tileset that contains this tile, or nullptr if not found in @p tilesets. */
        [[nodiscard]] TilemapTileset* GetTileset(const TilemapTilesetCollection& tilesets) const;

        /** @brief Gets the local tile ID within the tileset that contains this tile. */
        [[nodiscard]] int GetLocalId(const TilemapTilesetCollection& tilesets) const;

        /** @brief Gets the local tile ID within the tileset that contains this tile, also yielding that tileset (nullptr for the empty-tile sentinel). */
        [[nodiscard]] int GetLocalId(const TilemapTilesetCollection& tilesets, TilemapTileset*& tileset) const;

        /** @brief Gets the tile data for this tile from the tileset that contains it, or nullptr if not found or the tile has no data defined. */
        [[nodiscard]] const TilemapTileData* GetTileData(const TilemapTilesetCollection& tilesets) const;

    private:
        int globalId_ = 0;
        TilemapTileFlipFlags flipFlags_ = TilemapTileFlipFlags::None;
    };
}
