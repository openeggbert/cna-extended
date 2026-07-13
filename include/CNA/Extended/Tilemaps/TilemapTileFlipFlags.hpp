// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileFlipFlags.cs. `[Flags]` bitwise operators
// -> free function operators, matching this project's established pattern (see
// Graphics/FlipFlags.hpp).
#pragma once

#include <cstdint>
#include <type_traits>

namespace CNA::Extended::Tilemaps
{
    /** @brief Flip transformations applied to a tile. */
    enum class TilemapTileFlipFlags : std::uint8_t
    {
        None = 0,
        FlipHorizontally = 1,
        FlipVertically = 2,
        FlipDiagonally = 4
    };

    [[nodiscard]] constexpr TilemapTileFlipFlags operator|(TilemapTileFlipFlags left, TilemapTileFlipFlags right)
    {
        using U = std::underlying_type_t<TilemapTileFlipFlags>;
        return static_cast<TilemapTileFlipFlags>(static_cast<U>(left) | static_cast<U>(right));
    }

    [[nodiscard]] constexpr TilemapTileFlipFlags operator&(TilemapTileFlipFlags left, TilemapTileFlipFlags right)
    {
        using U = std::underlying_type_t<TilemapTileFlipFlags>;
        return static_cast<TilemapTileFlipFlags>(static_cast<U>(left) & static_cast<U>(right));
    }

    constexpr TilemapTileFlipFlags& operator|=(TilemapTileFlipFlags& left, TilemapTileFlipFlags right)
    {
        left = left | right;
        return left;
    }
}
