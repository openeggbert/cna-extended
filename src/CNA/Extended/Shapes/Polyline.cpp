// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Shapes/Polyline.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Shapes
{
    Polyline::Polyline(std::vector<Vector2> points) : points_(std::move(points))
    {
    }

    float Polyline::getLeftProperty() const
    {
        return std::min_element(points_.begin(), points_.end(), [](const Vector2& a, const Vector2& b) { return a.X < b.X; })->X;
    }

    float Polyline::getTopProperty() const
    {
        return std::min_element(points_.begin(), points_.end(), [](const Vector2& a, const Vector2& b) { return a.Y < b.Y; })->Y;
    }

    float Polyline::getRightProperty() const
    {
        return std::max_element(points_.begin(), points_.end(), [](const Vector2& a, const Vector2& b) { return a.X < b.X; })->X;
    }

    float Polyline::getBottomProperty() const
    {
        return std::max_element(points_.begin(), points_.end(), [](const Vector2& a, const Vector2& b) { return a.Y < b.Y; })->Y;
    }

    RectangleF Polyline::getBoundingRectangleProperty() const
    {
        const float minX = getLeftProperty();
        const float minY = getTopProperty();
        const float maxX = getRightProperty();
        const float maxY = getBottomProperty();
        return RectangleF(minX, minY, maxX - minX, maxY - minY);
    }

    bool Polyline::Contains(float, float) const
    {
        return false;
    }

    bool Polyline::Contains(const Vector2& point) const
    {
        return Contains(point.X, point.Y);
    }
}
