// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapDataLayer.cs. A layer that
// carries data but has no visual tiles; the renderer skips data layers entirely. Data layers
// carry format-specific information that does not correspond to renderable content (e.g.
// collision-type grids or region-tag grids). The data is stored in TilemapLayer's Properties;
// its meaning is defined by the source format and the game -- this project does not interpret
// the property values.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"

namespace CNA::Extended::Tilemaps
{
    /** @brief A layer that carries data but has no visual tiles. The renderer skips data layers entirely. */
    class TilemapDataLayer : public TilemapLayer
    {
    public:
        /** @brief Creates a data layer with @p name and grid dimensions @p width x @p height (each cell @p tileWidth x @p tileHeight pixels). */
        TilemapDataLayer(std::string name, int width, int height, int tileWidth, int tileHeight)
            : TilemapLayer(std::move(name)), width_(width), height_(height), tileWidth_(tileWidth), tileHeight_(tileHeight)
        {
        }

        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }
        [[nodiscard]] int getTileWidthProperty() const { return tileWidth_; }
        [[nodiscard]] int getTileHeightProperty() const { return tileHeight_; }

        [[nodiscard]] Rectangle getBoundsProperty() const override
        {
            return Rectangle(0, 0, width_ * tileWidth_, height_ * tileHeight_);
        }

    private:
        int width_;
        int height_;
        int tileWidth_;
        int tileHeight_;
    };
}
