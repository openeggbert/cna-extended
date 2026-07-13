// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapEllipseObject.hpp"

#include <cmath>

namespace CNA::Extended::Tilemaps
{
    TilemapEllipseObject::TilemapEllipseObject(int id, const Vector2& position, const Vector2& size) : TilemapObject(id, position), size_(size)
    {
    }

    std::optional<BoundingCircle2D> TilemapEllipseObject::getCircleProperty() const
    {
        if (std::abs(getRadiusXProperty() - getRadiusYProperty()) < 1e-6f)
        {
            return BoundingCircle2D(getCenterProperty(), getRadiusXProperty());
        }
        return std::nullopt;
    }

    BoundingBox2D TilemapEllipseObject::getBoundsProperty() const
    {
        return BoundingBox2D::CreateFromPositionAndSize(getPositionProperty(), size_);
    }
}
