// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileEntry.cs (`readonly record struct
// TilemapTileEntry(int X, int Y, TilemapTile Tile)`).
#pragma once

#include "CNA/Extended/Tilemaps/TilemapTile.hpp"

namespace CNA::Extended::Tilemaps
{
    /** @brief A tile instance at a specific (X, Y) position within a tile layer. */
    struct TilemapTileEntry
    {
        TilemapTileEntry() = default;

        TilemapTileEntry(int x, int y, const TilemapTile& tile) : X(x), Y(y), Tile(tile) {}

        int X = 0;
        int Y = 0;
        TilemapTile Tile;
    };
}
