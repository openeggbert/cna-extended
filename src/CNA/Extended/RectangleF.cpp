// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/RectangleF.hpp"

#include "CNA/Extended/MathExtended.hpp"
#include "CNA/Extended/PrimitivesHelper.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended
{
    const RectangleF RectangleF::Empty = RectangleF();

    RectangleF::RectangleF(const float x, const float y, const float width, const float height)
        : X(x), Y(y), Width(width), Height(height)
    {
    }

    RectangleF::RectangleF(const Vector2& position, const SizeF& size)
        : X(position.X), Y(position.Y), Width(size.Width), Height(size.Height)
    {
    }

    void RectangleF::CreateFrom(const Vector2& minimum, const Vector2& maximum, RectangleF& result)
    {
        result.X = minimum.X;
        result.Y = minimum.Y;
        result.Width = maximum.X - minimum.X;
        result.Height = maximum.Y - minimum.Y;
    }

    RectangleF RectangleF::CreateFrom(const Vector2& minimum, const Vector2& maximum)
    {
        RectangleF result;
        CreateFrom(minimum, maximum, result);
        return result;
    }

    void RectangleF::CreateFrom(const std::vector<Vector2>& points, RectangleF& result)
    {
        Vector2 minimum;
        Vector2 maximum;
        PrimitivesHelper::CreateRectangleFromPoints(points, minimum, maximum);
        CreateFrom(minimum, maximum, result);
    }

    RectangleF RectangleF::CreateFrom(const std::vector<Vector2>& points)
    {
        RectangleF result;
        CreateFrom(points, result);
        return result;
    }

    void RectangleF::Transform(RectangleF& rectangle, Matrix3x2& transformMatrix, RectangleF& result)
    {
        Vector2 center = rectangle.getCenterProperty();
        Vector2 halfExtents = Vector2(rectangle.Width, rectangle.Height) * 0.5f;
        PrimitivesHelper::TransformRectangle(center, halfExtents, transformMatrix);
        result.X = center.X - halfExtents.X;
        result.Y = center.Y - halfExtents.Y;
        result.Width = halfExtents.X * 2;
        result.Height = halfExtents.Y * 2;
    }

    RectangleF RectangleF::Transform(RectangleF rectangle, Matrix3x2& transformMatrix)
    {
        RectangleF result;
        Transform(rectangle, transformMatrix, result);
        return result;
    }

    void RectangleF::UpdateFromPoints(const std::vector<Vector2>& points)
    {
        const RectangleF rectangle = CreateFrom(points);
        X = rectangle.X;
        Y = rectangle.Y;
        Width = rectangle.Width;
        Height = rectangle.Height;
    }

    float RectangleF::SquaredDistanceTo(const Vector2& point) const
    {
        return PrimitivesHelper::SquaredDistanceToPointFromRectangle(getTopLeftProperty(), getBottomRightProperty(), point);
    }

    float RectangleF::DistanceTo(const Vector2& point) const
    {
        return std::sqrt(SquaredDistanceTo(point));
    }

    Vector2 RectangleF::ClosestPointTo(const Vector2& point) const
    {
        Vector2 result;
        PrimitivesHelper::ClosestPointToPointFromRectangle(getTopLeftProperty(), getBottomRightProperty(), point, result);
        return result;
    }

    void RectangleF::Union(const RectangleF& first, const RectangleF& second, RectangleF& result)
    {
        result.X = std::min(first.X, second.X);
        result.Y = std::min(first.Y, second.Y);
        result.Width = std::max(first.getRightProperty(), second.getRightProperty()) - result.X;
        result.Height = std::max(first.getBottomProperty(), second.getBottomProperty()) - result.Y;
    }

    RectangleF RectangleF::Union(const RectangleF& first, const RectangleF& second)
    {
        RectangleF result;
        Union(first, second, result);
        return result;
    }

    RectangleF RectangleF::Union(const RectangleF& rectangle) const
    {
        RectangleF result;
        Union(*this, rectangle, result);
        return result;
    }

    void RectangleF::Intersect(const RectangleF& value1, const RectangleF& value2, RectangleF& result)
    {
        const Vector2 firstMinimum = value1.getTopLeftProperty();
        const Vector2 firstMaximum = value1.getBottomRightProperty();
        const Vector2 secondMinimum = value2.getTopLeftProperty();
        const Vector2 secondMaximum = value2.getBottomRightProperty();

        const Vector2 minimum = MathExtended::CalculateMaximumVector2(firstMinimum, secondMinimum);
        const Vector2 maximum = MathExtended::CalculateMinimumVector2(firstMaximum, secondMaximum);

        if (maximum.X < minimum.X || maximum.Y < minimum.Y)
        {
            result = RectangleF();
        }
        else
        {
            result = CreateFrom(minimum, maximum);
        }
    }

    RectangleF RectangleF::Intersect(const RectangleF& value1, const RectangleF& value2)
    {
        RectangleF rectangle;
        Intersect(value1, value2, rectangle);
        return rectangle;
    }

    RectangleF RectangleF::Intersect(const RectangleF& rectangle) const
    {
        RectangleF result;
        Intersect(*this, rectangle, result);
        return result;
    }

    bool RectangleF::Intersects(const RectangleF& first, const RectangleF& second)
    {
        return first.X < second.X + second.Width && first.X + first.Width > second.X && first.Y < second.Y + second.Height &&
               first.Y + first.Height > second.Y;
    }

    bool RectangleF::Intersects(const RectangleF& rectangle) const
    {
        return Intersects(*this, rectangle);
    }

    void RectangleF::Normalize()
    {
        if (Width < 0)
        {
            X += Width;
            Width = -Width;
        }

        if (Height < 0)
        {
            Y += Height;
            Height = -Height;
        }
    }

    RectangleF RectangleF::Normalize(RectangleF rectangle)
    {
        if (rectangle.Width < 0)
        {
            rectangle.X += rectangle.Width;
            rectangle.Width = -rectangle.Width;
        }

        if (rectangle.Height < 0)
        {
            rectangle.Y += rectangle.Height;
            rectangle.Height = -rectangle.Height;
        }

        return rectangle;
    }

    void RectangleF::Normalize(const RectangleF& rectangle, RectangleF& result)
    {
        result.X = rectangle.X;
        result.Width = rectangle.Width;

        if (result.Width < 0)
        {
            result.X += result.Width;
            result.Width = -result.Width;
        }

        result.Y = rectangle.Y;
        result.Height = rectangle.Height;

        if (result.Height < 0)
        {
            result.Y += result.Height;
            result.Height = -result.Height;
        }
    }

    bool RectangleF::Contains(const RectangleF& rectangle, const Vector2& point)
    {
        return rectangle.X <= point.X && point.X < rectangle.X + rectangle.Width && rectangle.Y <= point.Y &&
               point.Y < rectangle.Y + rectangle.Height;
    }

    bool RectangleF::Contains(const Vector2& point) const
    {
        return Contains(*this, point);
    }

    void RectangleF::Inflate(const float horizontalAmount, const float verticalAmount)
    {
        X -= horizontalAmount;
        Y -= verticalAmount;
        Width += horizontalAmount * 2;
        Height += verticalAmount * 2;
    }

    void RectangleF::Offset(const float offsetX, const float offsetY)
    {
        X += offsetX;
        Y += offsetY;
    }

    void RectangleF::Offset(const Vector2& amount)
    {
        X += amount.X;
        Y += amount.Y;
    }

    bool RectangleF::Equals(const RectangleF& rectangle) const
    {
        return X == rectangle.X && Y == rectangle.Y && Width == rectangle.Width && Height == rectangle.Height;
    }

    int RectangleF::GetHashCode() const
    {
        int hashCode = static_cast<int>(std::hash<float>{}(X));
        hashCode = (hashCode * 397) ^ static_cast<int>(std::hash<float>{}(Y));
        hashCode = (hashCode * 397) ^ static_cast<int>(std::hash<float>{}(Width));
        hashCode = (hashCode * 397) ^ static_cast<int>(std::hash<float>{}(Height));
        return hashCode;
    }

    std::string RectangleF::ToString() const
    {
        return "X: " + std::to_string(X) + ", Y: " + std::to_string(Y) + ", Width: " + std::to_string(Width) +
               ", Height: " + std::to_string(Height);
    }

    RectangleF::RectangleF(const Rectangle& rectangle)
        : X(static_cast<float>(rectangle.X)), Y(static_cast<float>(rectangle.Y)), Width(static_cast<float>(rectangle.Width)),
          Height(static_cast<float>(rectangle.Height))
    {
    }

    RectangleF::operator Rectangle() const
    {
        return Rectangle(static_cast<int>(X), static_cast<int>(Y), static_cast<int>(Width), static_cast<int>(Height));
    }
}
