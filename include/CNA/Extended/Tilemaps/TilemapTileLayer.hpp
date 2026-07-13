// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapTileLayer.cs. Upstream's 2D
// array `TilemapTile?[,] _tiles` -> a flat `std::vector<std::optional<TilemapTile>>` of size
// width*height, indexed `y * Width + x` -- a plain implementation detail (this layout is never
// observed externally), not something that needs to bit-for-bit match C#'s internal multi-
// dimensional array storage order. `IEnumerable<TilemapTileEntry> GetTiles()`/`GetTilesInRegion`
// (C# generator methods using `yield return`) become eagerly-evaluated `std::vector` results,
// matching this project's established convention for generator-style query methods.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTile.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileEntry.hpp"

#include <optional>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A layer composed of a grid of tiles. */
    class TilemapTileLayer : public TilemapLayer
    {
    public:
        /** @brief Creates an empty (all-tiles-unset) tile layer with @p name and grid dimensions @p width x @p height (each cell @p tileWidth x @p tileHeight pixels). */
        TilemapTileLayer(std::string name, int width, int height, int tileWidth, int tileHeight);

        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }
        [[nodiscard]] int getTileWidthProperty() const { return tileWidth_; }
        [[nodiscard]] int getTileHeightProperty() const { return tileHeight_; }

        [[nodiscard]] Rectangle getBoundsProperty() const override
        {
            return Rectangle(0, 0, width_ * tileWidth_, height_ * tileHeight_);
        }

        /** @brief Gets the tile at (@p x, @p y), or std::nullopt if empty or out of bounds. */
        [[nodiscard]] std::optional<TilemapTile> GetTile(int x, int y) const;

        /**
         * @brief Sets the tile at (@p x, @p y). Pass std::nullopt to clear.
         * @throws System::ArgumentOutOfRangeException @p x or @p y is outside the layer's bounds.
         */
        void SetTile(int x, int y, const std::optional<TilemapTile>& tile);

        /** @brief Gets all non-empty tiles in the layer, in row-major (y outer, x inner) order. */
        [[nodiscard]] std::vector<TilemapTileEntry> GetTiles() const;

        /** @brief Gets all non-empty tiles within @p region (in tile coordinates), clamped to the layer's bounds. */
        [[nodiscard]] std::vector<TilemapTileEntry> GetTilesInRegion(const Rectangle& region) const;

    private:
        [[nodiscard]] std::size_t Index(int x, int y) const { return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x); }

        int width_;
        int height_;
        int tileWidth_;
        int tileHeight_;
        std::vector<std::optional<TilemapTile>> tiles_;
    };
}
