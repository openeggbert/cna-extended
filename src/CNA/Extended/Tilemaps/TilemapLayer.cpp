// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapLayer.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapLayer::TilemapLayer(std::string name) : name_(std::move(name))
    {
    }
}
