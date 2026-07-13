// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapRectangleObject.cs.
#pragma once

#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Tilemaps/TilemapObject.hpp"

namespace CNA::Extended::Tilemaps
{
    /** @brief A rectangular object in a tilemap. */
    class TilemapRectangleObject : public TilemapObject
    {
    public:
        /** @brief Creates a rectangle object with @p id, top-left @p position, and @p size. */
        TilemapRectangleObject(int id, const Vector2& position, const Vector2& size);

        /** @brief Gets/sets the size of the rectangle. */
        [[nodiscard]] const Vector2& getSizeProperty() const { return size_; }
        void setSizeProperty(const Vector2& value) { size_ = value; }

        /** @brief Gets the oriented bounding box representing the rectangle, accounting for rotation (pivots around Position, the top-left corner). */
        [[nodiscard]] OrientedBoundingBox2D getShapeProperty() const;

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override;

    private:
        Vector2 size_;
    };
}
