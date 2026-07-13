// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/CircleF.hpp"

#include "CNA/Extended/BoundingRectangle.hpp"
#include "CNA/Extended/MathExtended.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <numbers>

namespace CNA::Extended
{
    CircleF::CircleF(const Vector2& center, const float radius) : Center(center), Radius(radius)
    {
    }

    CircleF::CircleF(const Rectangle& rectangle)
    {
        const int halfWidth = rectangle.Width / 2;
        const int halfHeight = rectangle.Height / 2;
        Center = Vector2(static_cast<float>(rectangle.X + halfWidth), static_cast<float>(rectangle.Y + halfHeight));
        Radius = static_cast<float>(halfWidth > halfHeight ? halfWidth : halfHeight);
    }

    CircleF::CircleF(const RectangleF& rectangle)
    {
        const float halfWidth = rectangle.Width * 0.5f;
        const float halfHeight = rectangle.Height * 0.5f;
        Center = Vector2(rectangle.X + halfWidth, rectangle.Y + halfHeight);
        Radius = halfWidth > halfHeight ? halfWidth : halfHeight;
    }

    RectangleF CircleF::getBoundingRectangleProperty() const
    {
        const float minX = Center.X - Radius;
        const float minY = Center.Y - Radius;
        return RectangleF(minX, minY, getDiameterProperty(), getDiameterProperty());
    }

    float CircleF::getCircumferenceProperty() const
    {
        return 2.0f * std::numbers::pi_v<float> * Radius;
    }

    void CircleF::CreateFrom(const Vector2& minimum, const Vector2& maximum, CircleF& result)
    {
        result.Center = Vector2((maximum.X + minimum.X) * 0.5f, (maximum.Y + minimum.Y) * 0.5f);
        const Vector2 distanceVector = maximum - minimum;
        result.Radius = distanceVector.X > distanceVector.Y ? distanceVector.X * 0.5f : distanceVector.Y * 0.5f;
    }

    CircleF CircleF::CreateFrom(const Vector2& minimum, const Vector2& maximum)
    {
        CircleF result;
        CreateFrom(minimum, maximum, result);
        return result;
    }

    void CircleF::CreateFrom(const std::vector<Vector2>& points, CircleF& result)
    {
        if (points.empty())
        {
            result = CircleF();
            return;
        }

        Vector2 minimum(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vector2 maximum(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());

        for (auto index = static_cast<std::ptrdiff_t>(points.size()) - 1; index >= 0; --index)
        {
            const Vector2& point = points[static_cast<std::size_t>(index)];
            minimum = MathExtended::CalculateMinimumVector2(minimum, point);
            maximum = MathExtended::CalculateMaximumVector2(maximum, point);
        }

        CreateFrom(minimum, maximum, result);
    }

    CircleF CircleF::CreateFrom(const std::vector<Vector2>& points)
    {
        CircleF result;
        CreateFrom(points, result);
        return result;
    }

    bool CircleF::Intersects(const CircleF& first, const CircleF& second)
    {
        const Vector2 distanceVector = first.Center - second.Center;
        const float distanceSquared = Vector2::Dot(distanceVector, distanceVector);
        const float radiusSum = first.Radius + second.Radius;
        return distanceSquared <= radiusSum * radiusSum;
    }

    bool CircleF::Intersects(const CircleF& circle) const
    {
        return Intersects(*this, circle);
    }

    bool CircleF::Contains(const CircleF& circle, const Vector2& point)
    {
        const float dx = circle.Center.X - point.X;
        const float dy = circle.Center.Y - point.Y;
        const float d2 = dx * dx + dy * dy;
        const float r2 = circle.Radius * circle.Radius;
        return d2 <= r2;
    }

    bool CircleF::Contains(const Vector2& point) const
    {
        return Contains(*this, point);
    }

    Vector2 CircleF::ClosestPointTo(const Vector2& point) const
    {
        Vector2 distanceVector = point - Center;
        const float lengthSquared = Vector2::Dot(distanceVector, distanceVector);
        if (lengthSquared <= Radius * Radius)
        {
            return point;
        }
        distanceVector.Normalize();
        return Center + Radius * distanceVector;
    }

    Vector2 CircleF::BoundaryPointAt(const float angle) const
    {
        const Vector2 direction(std::cos(angle), std::sin(angle));
        return Center + Radius * direction;
    }

    bool CircleF::Equals(const CircleF& circle) const
    {
        return circle.Center == Center && circle.Radius == Radius;
    }

    int CircleF::GetHashCode() const
    {
        // Matches upstream's `unchecked { (Center.GetHashCode() * 397) ^ Radius.GetHashCode(); }`.
        return (Center.GetHashCode() * 397) ^ static_cast<int>(std::hash<float>{}(Radius));
    }

    std::string CircleF::ToString() const
    {
        return "Centre: " + Center.ToString() + ", Radius: " + std::to_string(Radius);
    }

    CircleF::operator Rectangle() const
    {
        const auto diameter = static_cast<int>(getDiameterProperty());
        return Rectangle(static_cast<int>(Center.X - Radius), static_cast<int>(Center.Y - Radius), diameter, diameter);
    }

    CircleF::operator RectangleF() const
    {
        const float diameter = getDiameterProperty();
        return RectangleF(Center.X - Radius, Center.Y - Radius, diameter, diameter);
    }
}
