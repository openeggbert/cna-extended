// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Size.cs. The explicit conversion from SizeF is defined in
// Size.cpp (not inline here) since it needs SizeF's complete type; SizeF.hpp includes Size.hpp,
// not the other way around, to avoid a circular include -- Size.hpp only forward-declares SizeF.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Point;
    using SharpRuntime::intcs;

    class SizeF;

    /** @brief A two-dimensional size defined by two integers, a width and a height. */
    struct Size
    {
        /** @brief A Size with Width and Height equal to 0. */
        static const Size Empty;

        /** @brief The horizontal component of this Size. */
        intcs Width = 0;

        /** @brief The vertical component of this Size. */
        intcs Height = 0;

        Size() = default;

        /**
         * @brief Initializes a new Size from the specified dimensions.
         * @param width The width.
         * @param height The height.
         */
        Size(intcs width, intcs height);

        /** @brief Gets a value that indicates whether this Size is empty. */
        [[nodiscard]] bool getIsEmptyProperty() const { return Width == 0 && Height == 0; }

        [[nodiscard]] bool Equals(const Size& size) const { return Width == size.Width && Height == size.Height; }
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Size& first, const Size& second) { return first.Equals(second); }
        friend bool operator!=(const Size& first, const Size& second) { return !first.Equals(second); }

        /** @brief Vector addition of two Size structures. */
        friend Size operator+(const Size& first, const Size& second) { return Add(first, second); }
        /** @brief Vector subtraction of two Size structures. */
        friend Size operator-(const Size& first, const Size& second) { return Subtract(first, second); }
        friend Size operator/(const Size& size, intcs value) { return Size(size.Width / value, size.Height / value); }
        friend Size operator*(const Size& size, intcs value) { return Size(size.Width * value, size.Height * value); }

        /** @brief Vector addition of two Size structures. */
        [[nodiscard]] static Size Add(const Size& first, const Size& second);
        /** @brief Vector subtraction of two Size structures. */
        [[nodiscard]] static Size Subtract(const Size& first, const Size& second);

        /** @brief Implicit conversion from a Point to a Size. */
        Size(const Point& point) : Width(point.X), Height(point.Y) {} // NOLINT(*-explicit-constructor)

        /** @brief Implicit conversion from a Size to a Point. */
        [[nodiscard]] operator Point() const { return Point(Width, Height); } // NOLINT(*-explicit-constructor)

        /** @brief Explicit conversion from a SizeF to a Size (truncating). */
        explicit Size(const SizeF& size);
    };
}
