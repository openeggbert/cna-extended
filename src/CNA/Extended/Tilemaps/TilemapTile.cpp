// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTile.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "CNA/Extended/Tilemaps/TilemapTilesetCollection.hpp"

namespace CNA::Extended::Tilemaps
{
    TilemapTile::TilemapTile(int globalId, TilemapTileFlipFlags flipFlags) : globalId_(globalId), flipFlags_(flipFlags)
    {
    }

    TilemapTileset* TilemapTile::GetTileset(const TilemapTilesetCollection& tilesets) const
    {
        return tilesets.GetTilesetForGid(globalId_);
    }

    int TilemapTile::GetLocalId(const TilemapTilesetCollection& tilesets) const
    {
        TilemapTileset* tileset = nullptr;
        return tilesets.GetLocalId(globalId_, tileset);
    }

    int TilemapTile::GetLocalId(const TilemapTilesetCollection& tilesets, TilemapTileset*& tileset) const
    {
        return tilesets.GetLocalId(globalId_, tileset);
    }

    const TilemapTileData* TilemapTile::GetTileData(const TilemapTilesetCollection& tilesets) const
    {
        TilemapTileset* tileset = GetTileset(tilesets);
        if (tileset == nullptr)
        {
            return nullptr;
        }

        const int localId = globalId_ - tileset->getFirstGlobalIdProperty();
        return tileset->GetTileData(localId);
    }
}
