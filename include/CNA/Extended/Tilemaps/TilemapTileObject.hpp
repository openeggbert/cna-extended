// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapTileObject.cs.
#pragma once

#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTile.hpp"

namespace CNA::Extended::Tilemaps
{
    /** @brief An object that displays a tile from a tileset. */
    class TilemapTileObject : public TilemapObject
    {
    public:
        /** @brief Creates a tile object with @p id, top-left @p position, @p tile data, and @p size. */
        TilemapTileObject(int id, const Vector2& position, const TilemapTile& tile, const Vector2& size);

        [[nodiscard]] const TilemapTile& getTileProperty() const { return tile_; }
        void setTileProperty(const TilemapTile& value) { tile_ = value; }

        [[nodiscard]] const Vector2& getSizeProperty() const { return size_; }
        void setSizeProperty(const Vector2& value) { size_ = value; }

        /** @brief Gets the oriented bounding box representing the tile object, accounting for rotation (pivots around Position, the top-left corner). */
        [[nodiscard]] OrientedBoundingBox2D getShapeProperty() const;

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override;

    private:
        TilemapTile tile_;
        Vector2 size_;
    };
}
