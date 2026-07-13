// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTextObject.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapTextObject::TilemapTextObject(int id, const Vector2& position, const Vector2& size, std::string text)
        : TilemapObject(id, position), size_(size), text_(std::move(text))
    {
    }
}
