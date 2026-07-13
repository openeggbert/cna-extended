// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTilesetCollection.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "System/InvalidOperationException.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Tilemaps
{
    TilemapTileset* TilemapTilesetCollection::operator[](int index) const
    {
        return tilesets_[static_cast<std::size_t>(index)].get();
    }

    void TilemapTilesetCollection::Add(std::unique_ptr<TilemapTileset> tileset)
    {
        if (tileset == nullptr)
        {
            throw std::invalid_argument("tileset must not be null.");
        }

        tilesets_.push_back(std::move(tileset));
    }

    bool TilemapTilesetCollection::Remove(const TilemapTileset* tileset)
    {
        const auto it = std::find_if(tilesets_.begin(), tilesets_.end(),
            [tileset](const std::unique_ptr<TilemapTileset>& candidate) { return candidate.get() == tileset; });
        if (it == tilesets_.end())
        {
            return false;
        }
        tilesets_.erase(it);
        return true;
    }

    TilemapTileset* TilemapTilesetCollection::GetTilesetForGid(int globalTileId) const
    {
        if (globalTileId == 0)
        {
            return nullptr;
        }

        for (const std::unique_ptr<TilemapTileset>& tileset : tilesets_)
        {
            if (tileset->ContainsGlobalId(globalTileId))
            {
                return tileset.get();
            }
        }

        return nullptr;
    }

    int TilemapTilesetCollection::GetLocalId(int globalTileId, TilemapTileset*& tileset) const
    {
        if (globalTileId == 0)
        {
            tileset = nullptr;
            return 0;
        }

        TilemapTileset* foundTileset = GetTilesetForGid(globalTileId);

        if (foundTileset == nullptr)
        {
            throw System::InvalidOperationException("No tileset found for global tile ID " + std::to_string(globalTileId) + ".");
        }

        tileset = foundTileset;
        return globalTileId - foundTileset->getFirstGlobalIdProperty();
    }
}
