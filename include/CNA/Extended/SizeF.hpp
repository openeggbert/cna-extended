// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/SizeF.cs. Upstream has a commented-out (dead) implicit
// operator to a "Size2" type that never shipped in this form -- not ported, matching upstream's
// own decision to leave it commented out rather than active.
#pragma once

#include "CNA/Extended/Size.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Vector2;

    /** @brief A two-dimensional size defined by two real numbers, a width and a height. */
    struct SizeF
    {
        /** @brief A SizeF with Width and Height equal to 0.0f. */
        static const SizeF Empty;

        /** @brief The horizontal component of this SizeF. */
        float Width = 0.0f;

        /** @brief The vertical component of this SizeF. */
        float Height = 0.0f;

        SizeF() = default;

        /**
         * @brief Initializes a new SizeF from the specified dimensions.
         * @param width The width.
         * @param height The height.
         */
        SizeF(float width, float height);

        /** @brief Gets a value that indicates whether this SizeF is empty. */
        [[nodiscard]] bool getIsEmptyProperty() const { return Width == 0 && Height == 0; }

        [[nodiscard]] bool Equals(const SizeF& size) const { return Width == size.Width && Height == size.Height; }
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const SizeF& first, const SizeF& second) { return first.Equals(second); }
        friend bool operator!=(const SizeF& first, const SizeF& second) { return !first.Equals(second); }

        /** @brief Vector addition of two SizeF structures. */
        friend SizeF operator+(const SizeF& first, const SizeF& second) { return Add(first, second); }
        /** @brief Vector subtraction of two SizeF structures. */
        friend SizeF operator-(const SizeF& first, const SizeF& second) { return Subtract(first, second); }
        /** @brief Subtracts a SizeF from a Vector2. */
        friend Vector2 operator-(const Vector2& first, const SizeF& second);
        friend SizeF operator/(const SizeF& size, float value) { return SizeF(size.Width / value, size.Height / value); }
        friend SizeF operator*(const SizeF& size, float value) { return SizeF(size.Width * value, size.Height * value); }

        /** @brief Vector addition of two SizeF structures. */
        [[nodiscard]] static SizeF Add(const SizeF& first, const SizeF& second);
        /** @brief Vector subtraction of two SizeF structures. */
        [[nodiscard]] static SizeF Subtract(const SizeF& first, const SizeF& second);

        /** @brief Implicit conversion from a Vector2 to a SizeF. */
        SizeF(const Vector2& point) : Width(point.X), Height(point.Y) {} // NOLINT(*-explicit-constructor)

        /** @brief Implicit conversion from a Point to a SizeF. */
        SizeF(const Point& point) : Width(static_cast<float>(point.X)), Height(static_cast<float>(point.Y)) {} // NOLINT(*-explicit-constructor)

        /** @brief Implicit conversion from a SizeF to a Vector2. */
        [[nodiscard]] operator Vector2() const { return Vector2(Width, Height); } // NOLINT(*-explicit-constructor)

        /** @brief Explicit conversion from a SizeF to a Point (truncating). */
        [[nodiscard]] explicit operator Point() const { return Point(static_cast<intcs>(Width), static_cast<intcs>(Height)); }
    };
}
