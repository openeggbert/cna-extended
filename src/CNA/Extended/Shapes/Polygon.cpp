// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Shapes/Polygon.hpp"

#include "System/HashCode.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace CNA::Extended::Shapes
{
    Polygon::Polygon(std::vector<Vector2> vertices) : localVertices_(std::move(vertices)), transformedVertices_(localVertices_)
    {
    }

    const std::vector<Vector2>& Polygon::getVerticesProperty() const
    {
        if (isDirty_)
        {
            transformedVertices_ = GetTransformedVertices();
            isDirty_ = false;
        }

        return transformedVertices_;
    }

    float Polygon::getLeftProperty() const
    {
        const auto& vertices = getVerticesProperty();
        return std::min_element(vertices.begin(), vertices.end(),
            [](const Vector2& a, const Vector2& b) { return a.X < b.X; })->X;
    }

    float Polygon::getRightProperty() const
    {
        const auto& vertices = getVerticesProperty();
        return std::max_element(vertices.begin(), vertices.end(),
            [](const Vector2& a, const Vector2& b) { return a.X < b.X; })->X;
    }

    float Polygon::getTopProperty() const
    {
        const auto& vertices = getVerticesProperty();
        return std::min_element(vertices.begin(), vertices.end(),
            [](const Vector2& a, const Vector2& b) { return a.Y < b.Y; })->Y;
    }

    float Polygon::getBottomProperty() const
    {
        const auto& vertices = getVerticesProperty();
        return std::max_element(vertices.begin(), vertices.end(),
            [](const Vector2& a, const Vector2& b) { return a.Y < b.Y; })->Y;
    }

    void Polygon::Offset(const Vector2& amount)
    {
        offset_ = offset_ + amount;
        isDirty_ = true;
    }

    void Polygon::Rotate(const float amount)
    {
        rotation_ += amount;
        isDirty_ = true;
    }

    void Polygon::Scale(const Vector2& amount)
    {
        scale_ = scale_ + amount;
        isDirty_ = true;
    }

    std::vector<Vector2> Polygon::GetTransformedVertices() const
    {
        std::vector<Vector2> newVertices(localVertices_.size());
        const bool isScaled = !(scale_ == Vector2::One);

        for (std::size_t i = 0; i < localVertices_.size(); i++)
        {
            Vector2 p = localVertices_[i];

            if (isScaled)
            {
                p = p * scale_;
            }

            if (rotation_ != 0.0f)
            {
                const float cos = std::cos(rotation_);
                const float sin = std::sin(rotation_);
                p = Vector2(cos * p.X - sin * p.Y, sin * p.X + cos * p.Y);
            }

            newVertices[i] = p + offset_;
        }

        return newVertices;
    }

    Polygon Polygon::TransformedCopy(const Vector2& offset, const float rotation, const Vector2& scale) const
    {
        Polygon polygon(localVertices_);
        polygon.Offset(offset);
        polygon.Rotate(rotation);
        polygon.Scale(scale - Vector2::One);
        return Polygon(polygon.getVerticesProperty());
    }

    RectangleF Polygon::getBoundingRectangleProperty() const
    {
        const float minX = getLeftProperty();
        const float minY = getTopProperty();
        const float maxX = getRightProperty();
        const float maxY = getBottomProperty();
        return RectangleF(minX, minY, maxX - minX, maxY - minY);
    }

    bool Polygon::Contains(const Vector2& point) const
    {
        return Contains(point.X, point.Y);
    }

    bool Polygon::Contains(const float x, const float y) const
    {
        int intersects = 0;
        const auto& vertices = getVerticesProperty();

        for (std::size_t i = 0; i < vertices.size(); i++)
        {
            const float x1 = vertices[i].X;
            const float y1 = vertices[i].Y;
            const float x2 = vertices[(i + 1) % vertices.size()].X;
            const float y2 = vertices[(i + 1) % vertices.size()].Y;

            if (((y1 <= y && y < y2) || (y2 <= y && y < y1)) && (x < (x2 - x1) / (y2 - y1) * (y - y1) + x1))
            {
                intersects++;
            }
        }

        return (intersects & 1) == 1;
    }

    bool Polygon::Equals(const Polygon& other) const
    {
        const auto& mine = getVerticesProperty();
        const auto& theirs = other.getVerticesProperty();
        return std::equal(mine.begin(), mine.end(), theirs.begin(), theirs.end());
    }

    int Polygon::GetHashCode() const
    {
        int hash = 27;
        for (const Vector2& v : getVerticesProperty())
        {
            hash = hash + 13 * hash + v.GetHashCode();
        }
        return hash;
    }
}
