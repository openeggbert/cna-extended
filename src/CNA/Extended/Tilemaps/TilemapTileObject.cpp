// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTileObject.hpp"

#include <cmath>

namespace CNA::Extended::Tilemaps
{
    TilemapTileObject::TilemapTileObject(int id, const Vector2& position, const TilemapTile& tile, const Vector2& size)
        : TilemapObject(id, position), tile_(tile), size_(size)
    {
    }

    OrientedBoundingBox2D TilemapTileObject::getShapeProperty() const
    {
        const float cos = std::cos(getRotationProperty());
        const float sin = std::sin(getRotationProperty());
        const Vector2 halfExtents = size_ * 0.5f;

        const Vector2 center =
            getPositionProperty() + Vector2(halfExtents.X * cos - halfExtents.Y * sin, halfExtents.X * sin + halfExtents.Y * cos);

        return OrientedBoundingBox2D::CreateFromRotation(center, getRotationProperty(), halfExtents);
    }

    BoundingBox2D TilemapTileObject::getBoundsProperty() const
    {
        return BoundingBox2D::CreateFromPoints(getShapeProperty().GetCorners());
    }
}
