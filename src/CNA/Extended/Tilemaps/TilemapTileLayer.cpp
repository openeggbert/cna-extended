// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"

#include "System/ArgumentOutOfRangeException.hpp"

#include <algorithm>
#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapTileLayer::TilemapTileLayer(std::string name, int width, int height, int tileWidth, int tileHeight)
        : TilemapLayer(std::move(name)),
          width_(width),
          height_(height),
          tileWidth_(tileWidth),
          tileHeight_(tileHeight),
          tiles_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height))
    {
    }

    std::optional<TilemapTile> TilemapTileLayer::GetTile(int x, int y) const
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
        {
            return std::nullopt;
        }

        return tiles_[Index(x, y)];
    }

    void TilemapTileLayer::SetTile(int x, int y, const std::optional<TilemapTile>& tile)
    {
        if (x < 0 || x >= width_)
        {
            throw System::ArgumentOutOfRangeException("x");
        }

        if (y < 0 || y >= height_)
        {
            throw System::ArgumentOutOfRangeException("y");
        }

        tiles_[Index(x, y)] = tile;
    }

    std::vector<TilemapTileEntry> TilemapTileLayer::GetTiles() const
    {
        std::vector<TilemapTileEntry> result;
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                const std::optional<TilemapTile>& tile = tiles_[Index(x, y)];
                if (tile.has_value())
                {
                    result.emplace_back(x, y, *tile);
                }
            }
        }
        return result;
    }

    std::vector<TilemapTileEntry> TilemapTileLayer::GetTilesInRegion(const Rectangle& region) const
    {
        const int startX = std::max(0, region.X);
        const int startY = std::max(0, region.Y);
        const int endX = std::min(width_, region.X + region.Width);
        const int endY = std::min(height_, region.Y + region.Height);

        std::vector<TilemapTileEntry> result;
        for (int y = startY; y < endY; ++y)
        {
            for (int x = startX; x < endX; ++x)
            {
                const std::optional<TilemapTile>& tile = tiles_[Index(x, y)];
                if (tile.has_value())
                {
                    result.emplace_back(x, y, *tile);
                }
            }
        }
        return result;
    }
}
