// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapWorld.cs. Constructed from a list of
// already-built `Tilemap` levels handed over by a parser -- following the same
// canonical-single-owner reasoning already established for `TilemapTilesetCollection`
// (nothing else keeps a reference to a level once it's handed to a world), `Levels` is an
// owning `std::vector<std::unique_ptr<Tilemap>>`. Upstream's `ArgumentNullException.
// ThrowIfNull(levels)` checks that the *list reference itself* isn't null -- a concept with
// no C++ equivalent here, since a `std::vector` parameter can never be null (only empty), so
// it is not translated (matching this project's established convention for reference-type
// null checks that become unrepresentable once the parameter type can't be null).
#pragma once

#include "CNA/Extended/Tilemaps/Tilemap.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A collection of positioned tilemaps that together form a world map. */
    class TilemapWorld
    {
    public:
        /** @brief Creates a world owning @p levels. */
        explicit TilemapWorld(std::vector<std::unique_ptr<Tilemap>> levels);

        /** @brief Gets the levels that make up this world. */
        [[nodiscard]] const std::vector<std::unique_ptr<Tilemap>>& getLevelsProperty() const { return levels_; }

    private:
        std::vector<std::unique_ptr<Tilemap>> levels_;
    };
}
