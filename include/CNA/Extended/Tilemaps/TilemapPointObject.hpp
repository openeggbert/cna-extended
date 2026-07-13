// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapPointObject.cs.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapObject.hpp"

namespace CNA::Extended::Tilemaps
{
    /** @brief A point object in a tilemap with no spatial extent. */
    class TilemapPointObject : public TilemapObject
    {
    public:
        /** @brief Creates a point object with @p id at @p position. */
        TilemapPointObject(int id, const Vector2& position) : TilemapObject(id, position) {}

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override
        {
            return BoundingBox2D(getPositionProperty(), getPositionProperty());
        }
    };
}
