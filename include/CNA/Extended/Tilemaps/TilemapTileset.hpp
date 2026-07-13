// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileset.cs. `Dictionary<int, TilemapTileData>
// _tileData` owns each `TilemapTileData` (a C# reference type -- the dictionary is its canonical
// home); `List<TilemapTileData> _animatedTileData` aliases a subset of those SAME instances
// (added via `AddTileData`, never separately constructed), not copies. `TilemapTileData` itself
// must be move-only in C++ (it owns `std::unique_ptr<TilemapObject>` collision shapes -- see that
// type's header comment), which `sharp-runtime::Dictionary`'s copy-based `Add(const TKey&, const
// TValue&)` API cannot store; `std::unordered_map<int, std::unique_ptr<TilemapTileData>>` is used
// directly for `tileData_` instead (this project's own `Dictionary<K,V>` wrapper is still used
// elsewhere in this module wherever the value type is copyable). `animatedTileData_` holds
// non-owning `TilemapTileData*`, safe since it never outlives `tileData_` (both are members of the
// same `TilemapTileset` instance) -- the same non-owning-alias-into-an-owning-sibling-container
// pattern already used by `EntityManager::entityBag_`/`entityPool_` aliasing `allEntities_`.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A collection of tiles from a single texture atlas (or, for image-collection tilesets, from per-tile custom images). */
    class TilemapTileset
    {
    public:
        /** @brief Creates a tileset. @p columns of 0 marks an image-collection tileset (tile IDs are arbitrary, not sequential). */
        TilemapTileset(std::string name, Texture2D* texture, int tileWidth, int tileHeight, int tileCount, int columns, int spacing = 0,
            int margin = 0);

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        [[nodiscard]] int getTileWidthProperty() const { return tileWidth_; }
        [[nodiscard]] int getTileHeightProperty() const { return tileHeight_; }
        [[nodiscard]] int getTileCountProperty() const { return tileCount_; }
        [[nodiscard]] int getColumnsProperty() const { return columns_; }
        [[nodiscard]] int getSpacingProperty() const { return spacing_; }
        [[nodiscard]] int getMarginProperty() const { return margin_; }

        /** @brief Gets/sets the first global tile ID for tiles in this tileset (covers [FirstGlobalId, FirstGlobalId + TileCount - 1]). */
        [[nodiscard]] int getFirstGlobalIdProperty() const { return firstGlobalId_; }
        void setFirstGlobalIdProperty(int value) { firstGlobalId_ = value; }

        [[nodiscard]] Texture2D* getTextureProperty() const { return texture_; }

        /** @brief Gets/sets the offset applied when rendering tiles from this tileset. */
        [[nodiscard]] const Vector2& getTileOffsetProperty() const { return tileOffset_; }
        void setTileOffsetProperty(const Vector2& value) { tileOffset_ = value; }

        /** @brief Gets the custom properties of the tileset. */
        [[nodiscard]] TilemapProperties& getPropertiesProperty() { return properties_; }
        [[nodiscard]] const TilemapProperties& getPropertiesProperty() const { return properties_; }

        /** @brief Adds (taking ownership of) tile data, keyed by its own LocalId. Registers it as animated if it has an Animation with frames. */
        void AddTileData(std::unique_ptr<TilemapTileData> tileData);

        /** @brief Gets the tile data for local tile ID @p localId, or nullptr if not found. */
        [[nodiscard]] TilemapTileData* GetTileData(int localId) const;

        /** @brief Gets whether @p globalTileId falls within this tileset's [FirstGlobalId, FirstGlobalId + max local ID] range. */
        [[nodiscard]] bool ContainsGlobalId(int globalTileId) const;

        /** @brief Gets all tile data entries that have an animation defined. Empty if none. */
        [[nodiscard]] const std::vector<TilemapTileData*>& GetAnimatedTiles() const { return animatedTileData_; }

        /**
         * @brief Resolves the texture and source rectangle to use when rendering tile @p localId, applying current
         * animation-frame advancement and any per-tile custom image automatically.
         * @param texture Receives the texture to draw from.
         * @param sourceRect Receives the source rectangle within @p texture.
         */
        void GetRenderSource(int localId, Texture2D*& texture, Rectangle& sourceRect) const;

        /**
         * @brief Gets the source rectangle for tile @p localId within the shared texture atlas, accounting for spacing/margin.
         * @throws System::InvalidOperationException this is an image-collection tileset (Columns == 0) -- use GetRenderSource instead.
         */
        [[nodiscard]] Rectangle GetTileRegion(int localId) const;

    private:
        std::string name_;
        Texture2D* texture_;
        int tileWidth_;
        int tileHeight_;
        int tileCount_;
        int columns_;
        int spacing_;
        int margin_;
        int firstGlobalId_ = 0;
        Vector2 tileOffset_;
        TilemapProperties properties_;

        std::unordered_map<int, std::unique_ptr<TilemapTileData>> tileData_;
        std::vector<TilemapTileData*> animatedTileData_;
        int maxLocalId_;
    };
}
