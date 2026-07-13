// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTilesetCollection.cs. Verified via
// TilemapFactory.cs's real call site (`TilemapTileset tileset = BuildTileset(...);
// tilemap.Tilesets.Add(tileset);`) that nothing else ever keeps a separate reference to a
// freshly-built tileset -- this collection (referenced as `Tilemap.Tilesets`) is the sole,
// canonical owner in practice, just like C# GC keeps a tileset alive purely by virtue of this
// list referencing it. Translated as an owning `std::vector<std::unique_ptr<TilemapTileset>>`
// (an earlier draft of this file used a non-owning `TilemapTileset*` vector based on reading
// this file in isolation -- corrected after reading the actual construction call site, which is
// exactly the kind of check this project's ownership-decision convention calls for). Includes
// `TilemapTileset.hpp` in full (not just a forward declaration) since `std::unique_ptr<T>`'s
// destructor requires `T` to be complete wherever this class's own special member functions are
// instantiated (matching `TilemapLayerCollection.hpp`'s identical requirement for `TilemapLayer`).
#pragma once

#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief An owning, ordered collection of the tilesets used by a tilemap. */
    class TilemapTilesetCollection
    {
    public:
        TilemapTilesetCollection() = default;

        /** @brief Gets the tileset at @p index. */
        [[nodiscard]] TilemapTileset* operator[](int index) const;

        /** @brief Gets the number of tilesets in the collection. */
        [[nodiscard]] int getCountProperty() const { return static_cast<int>(tilesets_.size()); }

        /** @brief Adds (taking ownership of) @p tileset to the collection. @throws std::invalid_argument tileset is nullptr. */
        void Add(std::unique_ptr<TilemapTileset> tileset);

        /** @brief Removes the tileset matching @p tileset (by pointer identity) from the collection (O(n)). @return true if it was found and removed. */
        bool Remove(const TilemapTileset* tileset);

        /** @brief Removes all tilesets from the collection. */
        void Clear() { tilesets_.clear(); }

        /** @brief Gets the tileset containing @p globalTileId, or nullptr if the ID is 0 or no tileset contains it. */
        [[nodiscard]] TilemapTileset* GetTilesetForGid(int globalTileId) const;

        /**
         * @brief Gets the local tile ID within a tileset from @p globalTileId, also yielding that tileset (nullptr when @p globalTileId is 0).
         * @throws System::InvalidOperationException no tileset contains @p globalTileId (and it is nonzero).
         */
        [[nodiscard]] int GetLocalId(int globalTileId, TilemapTileset*& tileset) const;

        [[nodiscard]] auto begin() const { return tilesets_.begin(); }
        [[nodiscard]] auto end() const { return tilesets_.end(); }

    private:
        std::vector<std::unique_ptr<TilemapTileset>> tilesets_;
    };
}
