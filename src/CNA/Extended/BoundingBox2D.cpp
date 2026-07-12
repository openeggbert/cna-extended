// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingBox2D.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace CNA::Extended
{
    BoundingBox2D::BoundingBox2D(const Vector2& min, const Vector2& max) : Min(min), Max(max)
    {
    }

    BoundingBox2D BoundingBox2D::CreateFromMinMax(const Vector2& min, const Vector2& max)
    {
        return BoundingBox2D(min, max);
    }

    BoundingBox2D BoundingBox2D::CreateFromCenterAndExtents(const Vector2& center, const Vector2& halfExtents)
    {
        return BoundingBox2D(center - halfExtents, center + halfExtents);
    }

    BoundingBox2D BoundingBox2D::CreateFromPositionAndSize(const Vector2& position, const Vector2& size)
    {
        return BoundingBox2D(position, position + size);
    }

    BoundingBox2D BoundingBox2D::CreateFromPoints(const std::vector<Vector2>& points)
    {
        if (points.empty())
        {
            throw std::invalid_argument("Cannot create bounding box from empty point array.");
        }

        Vector2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vector2 max(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

        for (const Vector2& point : points)
        {
            if (point.X < min.X) min.X = point.X;
            if (point.Y < min.Y) min.Y = point.Y;
            if (point.X > max.X) max.X = point.X;
            if (point.Y > max.Y) max.Y = point.Y;
        }

        return BoundingBox2D(min, max);
    }

    BoundingBox2D BoundingBox2D::CreateMerged(const BoundingBox2D& original, const BoundingBox2D& additional)
    {
        BoundingBox2D result;
        result.Min.X = std::min(original.Min.X, additional.Min.X);
        result.Min.Y = std::min(original.Min.Y, additional.Min.Y);
        result.Max.X = std::max(original.Max.X, additional.Max.X);
        result.Max.Y = std::max(original.Max.Y, additional.Max.Y);
        return result;
    }

    std::vector<Vector2> BoundingBox2D::GetCorners() const
    {
        return {
            Vector2(Min.X, Min.Y),
            Vector2(Max.X, Min.Y),
            Vector2(Max.X, Max.Y),
            Vector2(Min.X, Max.Y),
        };
    }

    void BoundingBox2D::GetCorners(Vector2 corners[4]) const
    {
        corners[0] = Vector2(Min.X, Min.Y);
        corners[1] = Vector2(Max.X, Min.Y);
        corners[2] = Vector2(Max.X, Max.Y);
        corners[3] = Vector2(Min.X, Max.Y);
    }

    BoundingBox2D BoundingBox2D::Transform(const Matrix& matrix) const
    {
        const Vector2 transformedCenter = Vector2::Transform(getCenterProperty(), matrix);
        const Vector2 halfExtents = getHalfExtentsProperty();

        // For each axis, sum the absolute values of the transformed half-extents.
        // This gives us the new box that bounds the rotated original
        Vector2 newHalfExtents;
        newHalfExtents.X = std::abs(matrix.M11) * halfExtents.X + std::abs(matrix.M12) * halfExtents.Y;
        newHalfExtents.Y = std::abs(matrix.M21) * halfExtents.X + std::abs(matrix.M22) * halfExtents.Y;

        return CreateFromCenterAndExtents(transformedCenter, newHalfExtents);
    }

    BoundingBox2D BoundingBox2D::Translate(const Vector2& translation) const
    {
        return BoundingBox2D(Min + translation, Max + translation);
    }

    void BoundingBox2D::Deconstruct(Vector2& min, Vector2& max) const
    {
        min = Min;
        max = Max;
    }

    bool BoundingBox2D::Equals(const BoundingBox2D& other) const
    {
        return Min == other.Min && Max == other.Max;
    }

    int BoundingBox2D::GetHashCode() const
    {
        return Min.GetHashCode() ^ Max.GetHashCode();
    }

    std::string BoundingBox2D::ToString() const
    {
        return "{Min:" + Min.ToString() + " Max:" + Max.ToString() + "}";
    }
}
