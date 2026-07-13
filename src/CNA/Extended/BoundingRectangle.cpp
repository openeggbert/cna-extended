// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingRectangle.hpp"

#include "CNA/Extended/MathExtended.hpp"
#include "CNA/Extended/PrimitivesHelper.hpp"

#include <cmath>

namespace CNA::Extended
{
    const BoundingRectangle BoundingRectangle::Empty = BoundingRectangle();

    BoundingRectangle::BoundingRectangle(const Vector2& center, const SizeF& halfExtents)
        : Center(center), HalfExtents(halfExtents)
    {
    }

    void BoundingRectangle::CreateFrom(const Vector2& minimum, const Vector2& maximum, BoundingRectangle& result)
    {
        result.Center = Vector2((maximum.X + minimum.X) * 0.5f, (maximum.Y + minimum.Y) * 0.5f);
        result.HalfExtents = Vector2((maximum.X - minimum.X) * 0.5f, (maximum.Y - minimum.Y) * 0.5f);
    }

    BoundingRectangle BoundingRectangle::CreateFrom(const Vector2& minimum, const Vector2& maximum)
    {
        BoundingRectangle result;
        CreateFrom(minimum, maximum, result);
        return result;
    }

    void BoundingRectangle::CreateFrom(const std::vector<Vector2>& points, BoundingRectangle& result)
    {
        Vector2 minimum;
        Vector2 maximum;
        PrimitivesHelper::CreateRectangleFromPoints(points, minimum, maximum);
        CreateFrom(minimum, maximum, result);
    }

    BoundingRectangle BoundingRectangle::CreateFrom(const std::vector<Vector2>& points)
    {
        BoundingRectangle result;
        CreateFrom(points, result);
        return result;
    }

    void BoundingRectangle::Transform(BoundingRectangle& boundingRectangle, Matrix3x2& transformMatrix, BoundingRectangle& result)
    {
        PrimitivesHelper::TransformRectangle(boundingRectangle.Center, boundingRectangle.HalfExtents, transformMatrix);
        result.Center = boundingRectangle.Center;
        result.HalfExtents = boundingRectangle.HalfExtents;
    }

    BoundingRectangle BoundingRectangle::Transform(BoundingRectangle boundingRectangle, Matrix3x2& transformMatrix)
    {
        BoundingRectangle result;
        Transform(boundingRectangle, transformMatrix, result);
        return result;
    }

    void BoundingRectangle::UpdateFromPoints(const std::vector<Vector2>& points)
    {
        const BoundingRectangle boundingRectangle = CreateFrom(points);
        Center = boundingRectangle.Center;
        HalfExtents = boundingRectangle.HalfExtents;
    }

    float BoundingRectangle::SquaredDistanceTo(const Vector2& point) const
    {
        return PrimitivesHelper::SquaredDistanceToPointFromRectangle(Center - HalfExtents, Center + HalfExtents, point);
    }

    Vector2 BoundingRectangle::ClosestPointTo(const Vector2& point) const
    {
        Vector2 result;
        PrimitivesHelper::ClosestPointToPointFromRectangle(Center - HalfExtents, Center + HalfExtents, point, result);
        return result;
    }

    void BoundingRectangle::Union(const BoundingRectangle& first, const BoundingRectangle& second, BoundingRectangle& result)
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 6.5; Bounding Volume
        // Hierarchies - Merging Bounding Volumes. pg 267
        const Vector2 firstMinimum = first.Center - first.HalfExtents;
        const Vector2 firstMaximum = first.Center + first.HalfExtents;
        const Vector2 secondMinimum = second.Center - second.HalfExtents;
        const Vector2 secondMaximum = second.Center + second.HalfExtents;

        const Vector2 minimum = MathExtended::CalculateMinimumVector2(firstMinimum, secondMinimum);
        const Vector2 maximum = MathExtended::CalculateMaximumVector2(firstMaximum, secondMaximum);

        result = CreateFrom(minimum, maximum);
    }

    BoundingRectangle BoundingRectangle::Union(const BoundingRectangle& first, const BoundingRectangle& second)
    {
        BoundingRectangle result;
        Union(first, second, result);
        return result;
    }

    BoundingRectangle BoundingRectangle::Union(const BoundingRectangle& boundingRectangle) const
    {
        return Union(*this, boundingRectangle);
    }

    void BoundingRectangle::Intersection(const BoundingRectangle& first, const BoundingRectangle& second, BoundingRectangle& result)
    {
        const Vector2 firstMinimum = first.Center - first.HalfExtents;
        const Vector2 firstMaximum = first.Center + first.HalfExtents;
        const Vector2 secondMinimum = second.Center - second.HalfExtents;
        const Vector2 secondMaximum = second.Center + second.HalfExtents;

        const Vector2 minimum = MathExtended::CalculateMaximumVector2(firstMinimum, secondMinimum);
        const Vector2 maximum = MathExtended::CalculateMinimumVector2(firstMaximum, secondMaximum);

        if (maximum.X < minimum.X || maximum.Y < minimum.Y)
        {
            result = BoundingRectangle();
        }
        else
        {
            result = CreateFrom(minimum, maximum);
        }
    }

    BoundingRectangle BoundingRectangle::Intersection(const BoundingRectangle& first, const BoundingRectangle& second)
    {
        BoundingRectangle result;
        Intersection(first, second, result);
        return result;
    }

    BoundingRectangle BoundingRectangle::Intersection(const BoundingRectangle& boundingRectangle) const
    {
        BoundingRectangle result;
        Intersection(*this, boundingRectangle, result);
        return result;
    }

    bool BoundingRectangle::Intersects(const BoundingRectangle& first, const BoundingRectangle& second)
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 4.2; Bounding Volumes -
        // Axis-aligned Bounding Boxes (AABBs). pg 80
        const Vector2 distance = first.Center - second.Center;
        const Vector2 radii = first.HalfExtents + second.HalfExtents;
        return std::abs(distance.X) <= radii.X && std::abs(distance.Y) <= radii.Y;
    }

    bool BoundingRectangle::Intersects(const BoundingRectangle& boundingRectangle) const
    {
        return Intersects(*this, boundingRectangle);
    }

    bool BoundingRectangle::Contains(const BoundingRectangle& boundingRectangle, const Vector2& point)
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 4.2; Bounding Volumes -
        // Axis-aligned Bounding Boxes (AABBs). pg 78
        const Vector2 distance = boundingRectangle.Center - point;
        const Vector2& radii = boundingRectangle.HalfExtents;

        return std::abs(distance.X) <= radii.X && std::abs(distance.Y) <= radii.Y;
    }

    bool BoundingRectangle::Contains(const Vector2& point) const
    {
        return Contains(*this, point);
    }

    bool BoundingRectangle::Equals(const BoundingRectangle& boundingRectangle) const
    {
        return boundingRectangle.Center == Center && boundingRectangle.HalfExtents == HalfExtents;
    }

    int BoundingRectangle::GetHashCode() const
    {
        return (Center.GetHashCode() * 397) ^ HalfExtents.GetHashCode();
    }

    std::string BoundingRectangle::ToString() const
    {
        return "Centre: " + Center.ToString() + ", Radii: " + HalfExtents.ToString();
    }

    BoundingRectangle::operator Rectangle() const
    {
        const Vector2 minimum = Center - HalfExtents;
        return Rectangle(static_cast<int>(minimum.X), static_cast<int>(minimum.Y), static_cast<int>(HalfExtents.X) * 2,
            static_cast<int>(HalfExtents.Y) * 2);
    }

    BoundingRectangle::operator RectangleF() const
    {
        const Vector2 minimum = Center - HalfExtents;
        return RectangleF(minimum.X, minimum.Y, HalfExtents.X * 2, HalfExtents.Y * 2);
    }

    BoundingRectangle::BoundingRectangle(const Rectangle& rectangle)
    {
        const SizeF radii(static_cast<float>(rectangle.Width) * 0.5f, static_cast<float>(rectangle.Height) * 0.5f);
        const Vector2 centre(static_cast<float>(rectangle.X) + radii.Width, static_cast<float>(rectangle.Y) + radii.Height);
        Center = centre;
        HalfExtents = radii;
    }

    BoundingRectangle::BoundingRectangle(const RectangleF& rectangle)
    {
        const SizeF radii(rectangle.Width * 0.5f, rectangle.Height * 0.5f);
        const Vector2 centre(rectangle.X + radii.Width, rectangle.Y + radii.Height);
        Center = centre;
        HalfExtents = radii;
    }
}
