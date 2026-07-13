// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapPolygonObject.hpp"

#include "CNA/Extended/Vector2Extensions.hpp"

#include <algorithm>
#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapPolygonObject::TilemapPolygonObject(int id, const Vector2& position, std::vector<Vector2> points)
        : TilemapObject(id, position), points_(std::move(points))
    {
    }

    std::vector<Vector2> TilemapPolygonObject::getWorldPointsProperty() const
    {
        std::vector<Vector2> result(points_.size());
        for (std::size_t i = 0; i < points_.size(); ++i)
        {
            result[i] = getPositionProperty() + points_[i];
        }
        return result;
    }

    BoundingPolygon2D TilemapPolygonObject::getShapeProperty() const
    {
        std::vector<Vector2> verts = getWorldPointsProperty();

        float signedArea = 0.0f;
        for (std::size_t i = 0; i < verts.size(); ++i)
        {
            signedArea += PerpDot(verts[i], verts[(i + 1) % verts.size()]);
        }

        if (signedArea < 0.0f)
        {
            std::reverse(verts.begin(), verts.end());
        }

        return BoundingPolygon2D(verts);
    }

    BoundingBox2D TilemapPolygonObject::getBoundsProperty() const
    {
        if (points_.empty())
        {
            return BoundingBox2D(getPositionProperty(), getPositionProperty());
        }

        return BoundingBox2D::CreateFromPoints(getWorldPointsProperty());
    }
}
