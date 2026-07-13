// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileAnimationFrame.cs.
#pragma once

namespace CNA::Extended::Tilemaps
{
    /** @brief A single frame in a tile animation: a local tile ID and its display duration. */
    struct TilemapTileAnimationFrame
    {
        TilemapTileAnimationFrame() = default;

        /** @brief Creates a frame with the given @p tileId and @p duration (seconds). */
        TilemapTileAnimationFrame(int tileId, float duration) : TileId(tileId), Duration(duration) {}

        /** @brief The local tile ID for this frame. */
        int TileId = 0;

        /** @brief The duration of this frame in seconds. */
        float Duration = 0.0f;
    };
}
