// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapRectangleObject.hpp"

#include <cmath>

namespace CNA::Extended::Tilemaps
{
    TilemapRectangleObject::TilemapRectangleObject(int id, const Vector2& position, const Vector2& size) : TilemapObject(id, position), size_(size)
    {
    }

    OrientedBoundingBox2D TilemapRectangleObject::getShapeProperty() const
    {
        const float cos = std::cos(getRotationProperty());
        const float sin = std::sin(getRotationProperty());
        const Vector2 halfExtents = size_ * 0.5f;

        const Vector2 center =
            getPositionProperty() + Vector2(halfExtents.X * cos - halfExtents.Y * sin, halfExtents.X * sin + halfExtents.Y * cos);

        return OrientedBoundingBox2D::CreateFromRotation(center, getRotationProperty(), halfExtents);
    }

    BoundingBox2D TilemapRectangleObject::getBoundsProperty() const
    {
        return BoundingBox2D::CreateFromPoints(getShapeProperty().GetCorners());
    }
}
