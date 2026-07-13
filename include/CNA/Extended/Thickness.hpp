// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/Thickness.cs. Fidelity note: Deconstruct's parameter
// order is (top, right, bottom, left) -- NOT the same order as the (left, top, right, bottom)
// constructor. This looks like it could be a typo/inconsistency in upstream, but it is exactly
// what Thickness.cs's Deconstruct declares and assigns; preserved as-is.
#pragma once

#include "CNA/Extended/Size.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <string>
#include <vector>

namespace CNA::Extended
{
    using SharpRuntime::intcs;

    /** @brief Represents the thickness of a frame around a rectangle, in integer units per edge. */
    struct Thickness
    {
        Thickness() = default;

        /**
         * @brief Initializes a new Thickness with the same value on all four edges. Not marked
         * explicit: upstream has both this constructor and a separate `implicit operator
         * Thickness(int value)` that just forwards to it; one non-explicit C++ constructor here
         * plays both roles at once (see Size.hpp/Interval.hpp for the same pattern).
         */
        Thickness(intcs all); // NOLINT(*-explicit-constructor)

        /** @brief Initializes a new Thickness with symmetric left/right and top/bottom values. */
        Thickness(intcs leftRight, intcs topBottom);

        /** @brief Initializes a new Thickness with the specified edge values. */
        Thickness(intcs left, intcs top, intcs right, intcs bottom);

        [[nodiscard]] intcs getLeftProperty() const { return left_; }
        void setLeftProperty(const intcs value) { left_ = value; }

        [[nodiscard]] intcs getTopProperty() const { return top_; }
        void setTopProperty(const intcs value) { top_ = value; }

        [[nodiscard]] intcs getRightProperty() const { return right_; }
        void setRightProperty(const intcs value) { right_ = value; }

        [[nodiscard]] intcs getBottomProperty() const { return bottom_; }
        void setBottomProperty(const intcs value) { bottom_ = value; }

        /** @brief Gets the combined left + right thickness. */
        [[nodiscard]] intcs getWidthProperty() const { return left_ + right_; }
        /** @brief Gets the combined top + bottom thickness. */
        [[nodiscard]] intcs getHeightProperty() const { return top_ + bottom_; }
        /** @brief Gets a Size representing the combined width and height. */
        [[nodiscard]] Size getSizeProperty() const { return Size(getWidthProperty(), getHeightProperty()); }

        [[nodiscard]] bool Equals(const Thickness& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Thickness& left, const Thickness& right) { return left.Equals(right); }
        friend bool operator!=(const Thickness& left, const Thickness& right) { return !left.Equals(right); }

        /**
         * @brief Creates a Thickness from an array of 1, 2, or 4 values, matching upstream's
         * (all) / (leftRight, topBottom) / (left, top, right, bottom) constructor overloads.
         * @throws std::invalid_argument if @p values does not have 1, 2, or 4 elements.
         */
        [[nodiscard]] static Thickness FromValues(const std::vector<intcs>& values);

        /**
         * @brief Parses a Thickness from a comma/space-separated string of 1, 2, or 4 integers.
         * @throws std::invalid_argument if the parsed value count is not 1, 2, or 4, or the
         * string contains a non-integer token.
         */
        [[nodiscard]] static Thickness Parse(const std::string& value);

        /**
         * @brief Deconstructs this Thickness. Parameter order is (top, right, bottom, left) --
         * see this file's header comment.
         */
        void Deconstruct(intcs& top, intcs& right, intcs& bottom, intcs& left) const;

    private:
        intcs left_ = 0;
        intcs top_ = 0;
        intcs right_ = 0;
        intcs bottom_ = 0;
    };
}
