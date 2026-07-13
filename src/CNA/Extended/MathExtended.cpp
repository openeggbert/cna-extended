// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/MathExtended.hpp"

namespace CNA::Extended
{
    Vector2 MathExtended::CalculateMinimumVector2(const Vector2& first, const Vector2& second)
    {
        return Vector2(first.X < second.X ? first.X : second.X, first.Y < second.Y ? first.Y : second.Y);
    }

    void MathExtended::CalculateMinimumVector2(const Vector2& first, const Vector2& second, Vector2& result)
    {
        result.X = first.X < second.X ? first.X : second.X;
        result.Y = first.Y < second.Y ? first.Y : second.Y;
    }

    Vector2 MathExtended::CalculateMaximumVector2(const Vector2& first, const Vector2& second)
    {
        return Vector2(first.X > second.X ? first.X : second.X, first.Y > second.Y ? first.Y : second.Y);
    }

    void MathExtended::CalculateMaximumVector2(const Vector2& first, const Vector2& second, Vector2& result)
    {
        result.X = first.X > second.X ? first.X : second.X;
        result.Y = first.Y > second.Y ? first.Y : second.Y;
    }
}
