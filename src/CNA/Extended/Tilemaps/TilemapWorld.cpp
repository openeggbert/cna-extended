// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapWorld.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapWorld::TilemapWorld(std::vector<std::unique_ptr<Tilemap>> levels) : levels_(std::move(levels))
    {
    }
}
