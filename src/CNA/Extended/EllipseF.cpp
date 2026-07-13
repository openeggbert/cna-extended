// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/EllipseF.hpp"

#include <cmath>
#include <functional>

namespace CNA::Extended
{
    EllipseF::EllipseF(const Vector2& center, const float radiusX, const float radiusY)
        : center_(center), radiusX_(radiusX), radiusY_(radiusY)
    {
    }

    RectangleF EllipseF::getBoundingRectangleProperty() const
    {
        const float minX = getLeftProperty();
        const float minY = getTopProperty();
        const float maxX = getRightProperty();
        const float maxY = getBottomProperty();
        return RectangleF(minX, minY, maxX - minX, maxY - minY);
    }

    bool EllipseF::Contains(const float x, const float y) const
    {
        const float xCalc = std::pow(x - center_.X, 2.0f) / std::pow(radiusX_, 2.0f);
        const float yCalc = std::pow(y - center_.Y, 2.0f) / std::pow(radiusY_, 2.0f);

        return xCalc + yCalc <= 1.0f;
    }

    bool EllipseF::Contains(const Vector2& point) const
    {
        return Contains(point.X, point.Y);
    }

    bool EllipseF::Equals(const EllipseF& ellipse) const
    {
        return ellipse.center_ == center_ && ellipse.radiusX_ == radiusX_ && ellipse.radiusY_ == radiusY_;
    }

    int EllipseF::GetHashCode() const
    {
        // Matches upstream's `unchecked { hashCode = Center.GetHashCode(); hashCode = (hashCode * 397) ^ RadiusX...; }`.
        int hashCode = center_.GetHashCode();
        hashCode = (hashCode * 397) ^ static_cast<int>(std::hash<float>{}(radiusX_));
        hashCode = (hashCode * 397) ^ static_cast<int>(std::hash<float>{}(radiusY_));
        return hashCode;
    }

    std::string EllipseF::ToString() const
    {
        return "Centre: " + center_.ToString() + ", RadiusX: " + std::to_string(radiusX_) +
               ", RadiusY: " + std::to_string(radiusY_);
    }
}
