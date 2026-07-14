// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Tilemap3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D/voxel counterpart of CNA::Extended::Tilemaps::Tilemap/TilemapTileLayer, deliberately
// scoped down per this task's own wording ("decide the exact shape during this phase based
// on what a real voxel/block world actually needs" -- plan3d.md's Phase 8 entry):
//
//  - A single sparse (X, Y, Z) -> tile ID grid, not upstream's multi-TilemapTileLayer stack
//    (background/foreground/collision layers etc.) -- a real voxel/block world (Minecraft-
//    style) is a single 3D grid, not a stack of 2D layers; layering isn't the right 3D
//    analog of TilemapTileLayer here. Sparse (std::unordered_map keyed by cell), not a
//    dense 3D array, because real voxel worlds are overwhelmingly empty space (most of a
//    Minecraft-style world above/below ground is air) -- a dense array would waste memory
//    proportional to the full bounding volume regardless of how much is actually built.
//  - Tile ID 0 means "empty" (air), matching the common voxel-engine convention (and this
//    project's own TilemapTileData convention of treating GID 0 as "no tile").
#pragma once

#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <cstddef>
#include <functional>
#include <unordered_map>

namespace CNA::Extended::World3DEXT
{
    /** @brief An integer 3D cell coordinate, used as Tilemap3DEXT's grid key. */
    struct TileCoordinate3DEXT
    {
        int X = 0;
        int Y = 0;
        int Z = 0;

        TileCoordinate3DEXT() = default;
        TileCoordinate3DEXT(int x, int y, int z) : X(x), Y(y), Z(z) {}

        [[nodiscard]] bool Equals(const TileCoordinate3DEXT& other) const { return X == other.X && Y == other.Y && Z == other.Z; }

        friend bool operator==(const TileCoordinate3DEXT& left, const TileCoordinate3DEXT& right) { return left.Equals(right); }
        friend bool operator!=(const TileCoordinate3DEXT& left, const TileCoordinate3DEXT& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::World3DEXT::TileCoordinate3DEXT>
    {
        std::size_t operator()(const CNA::Extended::World3DEXT::TileCoordinate3DEXT& value) const noexcept;
    };
}

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief A sparse 3D grid of tile IDs (0 == empty), addressed by integer cell
     * coordinates and rendered as uniformly-sized cubes.
     * @see Tilemap3DFactoryEXT for building one from hand-authored data.
     * @see TilemapRenderer3DEXT, which draws every non-empty cell as a cube.
     */
    class Tilemap3DEXT
    {
    public:
        /** @param tileSize The world-space size of one tile cell along each axis. Must have positive components. */
        explicit Tilemap3DEXT(const Microsoft::Xna::Framework::Vector3& tileSize = Microsoft::Xna::Framework::Vector3(1.0f, 1.0f, 1.0f));

        /** @brief Gets the world-space size of one tile cell along each axis. */
        [[nodiscard]] const Microsoft::Xna::Framework::Vector3& getTileSizeProperty() const { return tileSize_; }

        /** @brief Gets the tile ID at (x, y, z), or 0 if empty. */
        [[nodiscard]] int GetTileEXT(int x, int y, int z) const;

        /** @brief Sets the tile ID at (x, y, z). Setting 0 is equivalent to RemoveTileEXT. */
        void SetTileEXT(int x, int y, int z, int tileId);

        /** @brief Removes any tile at (x, y, z), leaving it empty. */
        void RemoveTileEXT(int x, int y, int z);

        /** @brief Gets whether (x, y, z) has a non-empty tile. */
        [[nodiscard]] bool HasTileEXT(int x, int y, int z) const;

        /** @brief Gets the number of non-empty tiles in this map. */
        [[nodiscard]] std::size_t GetTileCountEXT() const { return tiles_.size(); }

        /** @brief Gets every non-empty (coordinate, tile ID) pair. */
        [[nodiscard]] const std::unordered_map<TileCoordinate3DEXT, int>& getTilesProperty() const { return tiles_; }

        /** @brief Gets the world-space center of the tile cell at (x, y, z), regardless of whether it's occupied. */
        [[nodiscard]] Microsoft::Xna::Framework::Vector3 TileToWorldPositionEXT(int x, int y, int z) const;

    private:
        Microsoft::Xna::Framework::Vector3 tileSize_;
        std::unordered_map<TileCoordinate3DEXT, int> tiles_;
    };
}
