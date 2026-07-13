// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapPolylineObject.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapPolylineObject::TilemapPolylineObject(int id, const Vector2& position, std::vector<Vector2> points)
        : TilemapObject(id, position), points_(std::move(points))
    {
    }

    std::vector<Vector2> TilemapPolylineObject::getWorldPointsProperty() const
    {
        std::vector<Vector2> result(points_.size());
        for (std::size_t i = 0; i < points_.size(); ++i)
        {
            result[i] = getPositionProperty() + points_[i];
        }
        return result;
    }

    std::vector<LineSegment2D> TilemapPolylineObject::getSegmentsProperty() const
    {
        if (points_.size() < 2)
        {
            return {};
        }

        std::vector<LineSegment2D> segments(points_.size() - 1);
        for (std::size_t i = 0; i < points_.size() - 1; ++i)
        {
            segments[i] = LineSegment2D(getPositionProperty() + points_[i], getPositionProperty() + points_[i + 1]);
        }
        return segments;
    }

    BoundingBox2D TilemapPolylineObject::getBoundsProperty() const
    {
        if (points_.empty())
        {
            return BoundingBox2D(getPositionProperty(), getPositionProperty());
        }

        return BoundingBox2D::CreateFromPoints(getWorldPointsProperty());
    }
}
