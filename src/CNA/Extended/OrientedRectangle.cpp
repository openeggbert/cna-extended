// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/OrientedRectangle.hpp"

#include "CNA/Extended/PrimitivesHelper.hpp"
#include "System/HashCode.hpp"

#include <algorithm>
#include <array>

namespace CNA::Extended
{
    namespace
    {
        struct Extent
        {
            float Min;
            float Max;
        };

        Extent Project(const std::vector<Vector2>& vertices, const Vector2& axis)
        {
            float t = Vector2::Dot(vertices[0], axis);

            float min = t;
            float max = t;

            for (std::size_t c = 1; c < 4; c++)
            {
                t = Vector2::Dot(vertices[c], axis);

                if (t < min)
                {
                    min = t;
                }
                else if (t > max)
                {
                    max = t;
                }
            }

            return {min, max};
        }

        bool IsOverlapping(const Extent& p1, const Extent& p2)
        {
            return p1.Min <= p2.Max && p1.Max >= p2.Min;
        }

        float GetOverlap(const Extent& p1, const Extent& p2)
        {
            return std::min(p1.Max, p2.Max) - std::max(p1.Min, p2.Min);
        }
    }

    OrientedRectangle::OrientedRectangle(const Vector2& center, const SizeF& radii, const Matrix3x2& orientation)
        : Center(center), Radii(radii), Orientation(orientation)
    {
    }

    std::vector<Vector2> OrientedRectangle::getPointsProperty() const
    {
        const Vector2 topLeft = -Radii;
        const Vector2 bottomLeft = -Vector2(Radii.X, -Radii.Y);
        const Vector2 topRight = Vector2(Radii.X, -Radii.Y);
        const Vector2 bottomRight = Radii;

        return {
            Vector2::Transform(topRight, Orientation) + Center,
            Vector2::Transform(topLeft, Orientation) + Center,
            Vector2::Transform(bottomLeft, Orientation) + Center,
            Vector2::Transform(bottomRight, Orientation) + Center,
        };
    }

    Vector2 OrientedRectangle::getPositionProperty() const
    {
        return Vector2::Transform(-Radii, Orientation) + Center;
    }

    void OrientedRectangle::setPositionProperty(const Vector2&)
    {
        // Matches upstream: the Position setter is declared but always throws.
        throw std::logic_error("OrientedRectangle::setPositionProperty is not implemented (matches upstream MonoGame.Extended)");
    }

    RectangleF OrientedRectangle::getBoundingRectangleProperty() const
    {
        return static_cast<RectangleF>(*this);
    }

    OrientedRectangle OrientedRectangle::Transform(OrientedRectangle rectangle, Matrix3x2& transformMatrix)
    {
        // Matches upstream's private ref-taking overload, inlined here since it is not part of
        // upstream's public API contract (see header comment).
        PrimitivesHelper::TransformOrientedRectangle(rectangle.Center, rectangle.Orientation, transformMatrix);
        OrientedRectangle result;
        result.Center = rectangle.Center;
        result.Radii = rectangle.Radii;
        result.Orientation = rectangle.Orientation;
        return result;
    }

    bool OrientedRectangle::Equals(const OrientedRectangle& other) const
    {
        return Center == other.Center && Radii == other.Radii && Orientation == other.Orientation;
    }

    int OrientedRectangle::GetHashCode() const
    {
        return System::HashCode::Combine(Center.GetHashCode(), Radii.GetHashCode(), Orientation.GetHashCode());
    }

    std::string OrientedRectangle::ToString() const
    {
        return "Centre: " + Center.ToString() + ", Radii: " + Radii.ToString() + ", Orientation: " + Orientation.ToString();
    }

    OrientedRectangle::OrientedRectangle(const RectangleF& rectangle)
    {
        const SizeF radii(rectangle.Width * 0.5f, rectangle.Height * 0.5f);
        const Vector2 centre(rectangle.X + radii.Width, rectangle.Y + radii.Height);

        Center = centre;
        Radii = radii;
        Orientation = Matrix3x2::Identity;
    }

    OrientedRectangle::operator RectangleF() const
    {
        const Vector2 topLeft = -Radii;
        RectangleF rectangle(topLeft, Radii * 2.0f);
        Matrix3x2 orientation = Orientation * Matrix3x2::CreateTranslation(Center);
        return RectangleF::Transform(rectangle, orientation);
    }

    OrientedRectangleIntersection OrientedRectangle::Intersects(const OrientedRectangle& rectangle, const OrientedRectangle& other)
    {
        const std::vector<Vector2> corners = rectangle.getPointsProperty();
        const std::vector<Vector2> otherCorners = other.getPointsProperty();

        const std::array<Vector2, 4> allAxis = {
            corners[1] - corners[0],
            corners[3] - corners[0],
            otherCorners[1] - otherCorners[0],
            otherCorners[3] - otherCorners[0],
        };

        std::array<Vector2, 4> normalizedAxis = allAxis;

        float overlap = 0.0f;
        Vector2 minimumTranslationVector = Vector2::Zero;

        // Make the length of each axis 1/edge length, so we know any
        // dot product must be less than 1 to fall within the edge.
        for (Vector2& axis : normalizedAxis)
        {
            axis = axis / axis.LengthSquared();
        }

        for (std::size_t a = 0; a < normalizedAxis.size(); a++)
        {
            const Vector2& axisProjectedOnto = normalizedAxis[a];
            const Vector2& originalAxis = allAxis[a];

            const Extent p1 = Project(corners, axisProjectedOnto);
            const Extent p2 = Project(otherCorners, axisProjectedOnto);

            if (!IsOverlapping(p1, p2))
            {
                // There was no intersection along this dimension;
                // the boxes cannot possibly overlap.
                return {false, Vector2::Zero};
            }

            const float o = GetOverlap(p1, p2);
            if (o < overlap || overlap == 0.0f)
            {
                overlap = o;
                minimumTranslationVector = originalAxis * overlap;
                if (p1.Min > p2.Min)
                {
                    minimumTranslationVector = -minimumTranslationVector;
                }
            }
        }

        // There was no dimension along which there is no intersection.
        // Therefore, the boxes overlap.
        return {true, minimumTranslationVector};
    }
}
