// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapOrientation.cs, which despite its filename
// declares three separate enums (TilemapStaggerAxis, TilemapStaggerIndex, TilemapOrientation) --
// all ported here together to match the upstream file's actual contents.
#pragma once

namespace CNA::Extended::Tilemaps
{
    /** @brief Which axis is staggered for staggered/hexagonal tilemaps. */
    enum class TilemapStaggerAxis
    {
        X,
        Y
    };

    /** @brief Which rows or columns are staggered for staggered/hexagonal tilemaps. */
    enum class TilemapStaggerIndex
    {
        Even,
        Odd
    };

    /** @brief The grid layout of a tilemap. */
    enum class TilemapOrientation
    {
        /** @brief Standard grid layout where tiles are arranged in rows and columns. */
        Orthogonal,
        /** @brief Diamond-shaped grid layout where tiles are rotated 45 degrees. */
        Isometric,
        /** @brief Hexagonal or isometric layout with staggered rows or columns. */
        Staggered,
        /** @brief Hexagonal grid layout. */
        Hexagonal
    };
}
