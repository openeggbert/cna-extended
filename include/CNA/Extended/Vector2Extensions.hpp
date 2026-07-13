// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Vector2Extensions.cs. C# extension methods have no C++
// equivalent syntax; ported as free functions in this namespace, matching the convention used
// throughout this project. Upstream's `#if FNA || KNI` (Rotate) and `#if FNA` (ToPoint/ToVector2)
// blocks are ported unconditionally -- CNA mirrors FNA, matching the precedent set by
// RectangleExtensions for its own FNA-conditional members.
#pragma once

#include "CNA/Extended/SizeF.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/MidpointRounding.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Computes the 2D pseudo cross product (perp-dot product) of two vectors: a scalar
     * value representing twice the signed area of the parallelogram formed by the vectors.
     */
    [[nodiscard]] float PerpDot(const Vector2& value1, const Vector2& value2);

    /** @brief Computes the 2D pseudo cross product (perp-dot product) of two vectors. */
    void PerpDot(const Vector2& value1, const Vector2& value2, float& result);

    /** @brief Returns a copy of @p vector2 with its X component replaced by @p x. */
    [[nodiscard]] Vector2 SetX(const Vector2& vector2, float x);

    /** @brief Returns a copy of @p vector2 with its Y component replaced by @p y. */
    [[nodiscard]] Vector2 SetY(const Vector2& vector2, float y);

    /** @brief Returns a copy of @p vector2 translated by the specified x/y amounts. */
    [[nodiscard]] Vector2 Translate(const Vector2& vector2, float x, float y);

    /** @brief Converts a Vector2 to a SizeF with the same X/Y values as Width/Height. */
    [[nodiscard]] SizeF ToSize(const Vector2& value);

    /** @brief Converts a Vector2 to a SizeF using the absolute value of its X/Y components. */
    [[nodiscard]] SizeF ToAbsoluteSize(const Vector2& value);

    /** @brief Rounds the components of @p value to @p digits decimal places using @p mode. */
    [[nodiscard]] Vector2 Round(const Vector2& value, int digits, System::MidpointRounding mode);

    /** @brief Rounds the components of @p value to @p digits decimal places. */
    [[nodiscard]] Vector2 Round(const Vector2& value, int digits);

    /** @brief Rounds the components of @p value to the nearest integer. */
    [[nodiscard]] Vector2 Round(const Vector2& value);

    /** @brief Determines whether @p value and @p otherValue are equal within @p tolerance. */
    [[nodiscard]] bool EqualsWithTolerence(const Vector2& value, const Vector2& otherValue, float tolerance = 0.00001f);

    /** @brief Rotates @p value by the specified angle, in radians. */
    [[nodiscard]] Vector2 Rotate(const Vector2& value, float radians);

    /** @brief Returns a normalized copy of @p value, leaving @p value itself unchanged. */
    [[nodiscard]] Vector2 NormalizedCopy(const Vector2& value);

    /** @brief Returns the vector perpendicular to @p value, rotated clockwise. */
    [[nodiscard]] Vector2 PerpendicularClockwise(const Vector2& value);

    /** @brief Returns the vector perpendicular to @p value, rotated counter-clockwise. */
    [[nodiscard]] Vector2 PerpendicularCounterClockwise(const Vector2& value);

    /** @brief Returns @p value truncated to at most @p maxLength in length. */
    [[nodiscard]] Vector2 Truncate(const Vector2& value, float maxLength);

    /** @brief Determines whether either component of @p value is NaN. */
    [[nodiscard]] bool IsNaN(const Vector2& value);

    /** @brief Converts @p value to an angle, in radians. */
    [[nodiscard]] float ToAngle(const Vector2& value);

    /** @brief Calculates the dot product of two vectors. */
    [[nodiscard]] float Dot(const Vector2& vector1, const Vector2& vector2);

    /** @brief Calculates the scalar projection of @p vector1 onto @p vector2. */
    [[nodiscard]] float ScalarProjectOnto(const Vector2& vector1, const Vector2& vector2);

    /** @brief Calculates the vector projection of @p vector1 onto @p vector2. */
    [[nodiscard]] Vector2 ProjectOnto(const Vector2& vector1, const Vector2& vector2);

    /** @brief Gets a Point representation for this Vector2 (matches upstream's `#if FNA` extension). */
    [[nodiscard]] Point ToPoint(const Vector2& value);

    /** @brief Gets a Vector2 representation for this Point (matches upstream's `#if FNA` extension). */
    [[nodiscard]] Vector2 ToVector2(const Point& value);
}
