// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapLayers/TilemapObjectDrawOrder.cs.
#pragma once

namespace CNA::Extended::Tilemaps
{
    /** @brief The rendering order for objects in an object layer. */
    enum class TilemapObjectDrawOrder
    {
        /** @brief Objects are drawn based on their Y position (top to bottom). */
        TopDown,
        /** @brief Objects are drawn in the order they appear in the layer. */
        Index
    };
}
