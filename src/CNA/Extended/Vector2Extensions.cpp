// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Vector2Extensions.hpp"

#include "System/Math.hpp"

#include <cmath>

namespace CNA::Extended
{
    float PerpDot(const Vector2& value1, const Vector2& value2)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 3.3.5 The Cross Product
        return (value1.X * value2.Y) - (value1.Y * value2.X);
    }

    void PerpDot(const Vector2& value1, const Vector2& value2, float& result)
    {
        result = (value1.X * value2.Y) - (value1.Y * value2.X);
    }

    Vector2 SetX(const Vector2& vector2, const float x)
    {
        return Vector2(x, vector2.Y);
    }

    Vector2 SetY(const Vector2& vector2, const float y)
    {
        return Vector2(vector2.X, y);
    }

    Vector2 Translate(const Vector2& vector2, const float x, const float y)
    {
        return Vector2(vector2.X + x, vector2.Y + y);
    }

    SizeF ToSize(const Vector2& value)
    {
        return SizeF(value.X, value.Y);
    }

    SizeF ToAbsoluteSize(const Vector2& value)
    {
        const float x = std::abs(value.X);
        const float y = std::abs(value.Y);
        return SizeF(x, y);
    }

    Vector2 Round(const Vector2& value, const int digits, const System::MidpointRounding mode)
    {
        const float x = static_cast<float>(System::Math::Round(static_cast<double>(value.X), digits, mode));
        const float y = static_cast<float>(System::Math::Round(static_cast<double>(value.Y), digits, mode));
        return Vector2(x, y);
    }

    Vector2 Round(const Vector2& value, const int digits)
    {
        const float x = static_cast<float>(System::Math::Round(static_cast<double>(value.X), digits));
        const float y = static_cast<float>(System::Math::Round(static_cast<double>(value.Y), digits));
        return Vector2(x, y);
    }

    Vector2 Round(const Vector2& value)
    {
        const float x = static_cast<float>(System::Math::Round(static_cast<double>(value.X)));
        const float y = static_cast<float>(System::Math::Round(static_cast<double>(value.Y)));
        return Vector2(x, y);
    }

    bool EqualsWithTolerence(const Vector2& value, const Vector2& otherValue, const float tolerance)
    {
        return std::abs(value.X - otherValue.X) <= tolerance && std::abs(value.Y - otherValue.Y) <= tolerance;
    }

    Vector2 Rotate(const Vector2& value, const float radians)
    {
        const float cos = std::cos(radians);
        const float sin = std::sin(radians);
        return Vector2(value.X * cos - value.Y * sin, value.X * sin + value.Y * cos);
    }

    Vector2 NormalizedCopy(const Vector2& value)
    {
        Vector2 newVector2(value.X, value.Y);
        newVector2.Normalize();
        return newVector2;
    }

    Vector2 PerpendicularClockwise(const Vector2& value)
    {
        return Vector2(value.Y, -value.X);
    }

    Vector2 PerpendicularCounterClockwise(const Vector2& value)
    {
        return Vector2(-value.Y, value.X);
    }

    Vector2 Truncate(const Vector2& value, const float maxLength)
    {
        if (value.LengthSquared() > maxLength * maxLength)
        {
            return NormalizedCopy(value) * maxLength;
        }

        return value;
    }

    bool IsNaN(const Vector2& value)
    {
        return std::isnan(value.X) || std::isnan(value.Y);
    }

    float ToAngle(const Vector2& value)
    {
        return std::atan2(value.X, -value.Y);
    }

    float Dot(const Vector2& vector1, const Vector2& vector2)
    {
        return vector1.X * vector2.X + vector1.Y * vector2.Y;
    }

    float ScalarProjectOnto(const Vector2& vector1, const Vector2& vector2)
    {
        const float dotNumerator = vector1.X * vector2.X + vector1.Y * vector2.Y;
        const float lengthSquaredDenominator = vector2.X * vector2.X + vector2.Y * vector2.Y;
        return dotNumerator / std::sqrt(lengthSquaredDenominator);
    }

    Vector2 ProjectOnto(const Vector2& vector1, const Vector2& vector2)
    {
        const float dotNumerator = vector1.X * vector2.X + vector1.Y * vector2.Y;
        const float lengthSquaredDenominator = vector2.X * vector2.X + vector2.Y * vector2.Y;
        return vector2 * (dotNumerator / lengthSquaredDenominator);
    }

    Point ToPoint(const Vector2& value)
    {
        return Point(static_cast<SharpRuntime::intcs>(value.X), static_cast<SharpRuntime::intcs>(value.Y));
    }

    Vector2 ToVector2(const Point& value)
    {
        return Vector2(static_cast<float>(value.X), static_cast<float>(value.Y));
    }
}
