// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's HslColor.cs. Upstream's Match(Action<float,float,float>) and
// Map<T>(Func<float,float,float,T>) callback-style accessors are ported using
// std::function-compatible templates (any callable works, matching C#'s delegate flexibility).
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <functional>
#include <stdexcept>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Color;

    /**
     * @brief Represents a color in the HSL (Hue, Saturation, Lightness) color space.
     *
     * Hue (H) represents the color, ranging from 0 to 360 degrees on the color wheel.
     * Saturation (S) represents the intensity of the color, ranging from 0.0 (gray) to 1.0
     * (full color). Lightness (L) represents the brightness, ranging from 0.0 (black) to 1.0
     * (white).
     */
    struct HslColor
    {
        HslColor() = default;

        /**
         * @brief Initializes a new HslColor with the specified hue, saturation, and lightness
         * component values.
         * @param h The hue component value (in degrees) from 0.0 to 360.0.
         * @param s The saturation component value from 0.0 to 1.0.
         * @param l The lightness component value from 0.0 to 1.0.
         */
        HslColor(float h, float s, float l);

        /** @brief The hue component value (in degrees), ranging from 0.0 to 360.0. */
        [[nodiscard]] float getHProperty() const { return h_; }

        /** @brief The saturation component value, ranging from 0.0 to 1.0. */
        [[nodiscard]] float getSProperty() const { return s_; }

        /** @brief The lightness component value, ranging from 0.0 to 1.0. */
        [[nodiscard]] float getLProperty() const { return l_; }

        /**
         * @brief Copies the value of this HslColor to an existing destination, modifying it
         * in-place rather than constructing a new instance.
         * @param destination The destination HslColor to copy into.
         */
        void CopyToRef(HslColor& destination) const;

        /**
         * @brief Deconstructs this HslColor into its hue, saturation, and lightness component
         * values.
         */
        void Deconstruct(float& h, float& s, float& l) const;

        /**
         * @brief Executes a callback with the components of this HslColor.
         * @param callback The callback to execute.
         */
        void Match(const std::function<void(float, float, float)>& callback) const;

        /**
         * @brief Maps the components of this HslColor to a new value using the specified
         * mapping function.
         * @tparam T The type of the result of the mapping function.
         * @param map The mapping function to apply to the components of this HslColor.
         */
        template <typename T>
        T Map(const std::function<T(float, float, float)>& map) const
        {
            if (!map)
            {
                throw std::invalid_argument("map");
            }
            return map(h_, s_, l_);
        }

        /**
         * @brief Compares this HslColor to another, ordering primarily by hue, then lightness,
         * then saturation.
         * @param other The HslColor to compare with.
         * @return A negative value if this instance precedes @p other, zero if they are equal
         * in ordering, or a positive value if this instance follows @p other.
         */
        [[nodiscard]] int CompareTo(const HslColor& other) const;

        [[nodiscard]] bool Equals(const HslColor& value) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        /**
         * @brief Parses a string into an HslColor. Expects the format "hue,saturation,lightness",
         * where hue is in degrees (optionally followed by the '°' symbol).
         * @param s The string to parse.
         */
        [[nodiscard]] static HslColor Parse(const std::string& s);

        friend bool operator==(const HslColor& x, const HslColor& y) { return x.Equals(y); }
        friend bool operator!=(const HslColor& x, const HslColor& y) { return !x.Equals(y); }

        friend HslColor operator+(const HslColor& a, const HslColor& b);
        friend HslColor operator-(const HslColor& a, const HslColor& b);

        /**
         * @brief Linearly interpolates between two HslColor values.
         * @param c1 The first HslColor.
         * @param c2 The second HslColor.
         * @param t The interpolation factor: 0 returns @p c1, 1 returns @p c2.
         */
        [[nodiscard]] static HslColor Lerp(const HslColor& c1, const HslColor& c2, float t);

        /**
         * @brief Converts an HslColor value to a Color value.
         * @param hsl The HslColor value to convert.
         */
        [[nodiscard]] static Color ToRgb(const HslColor& hsl);

        /**
         * @brief Converts an RGB Color to an equivalent HslColor.
         * @param color The RGB color to convert.
         */
        [[nodiscard]] static HslColor FromRgb(const Color& color);

    private:
        static float NormalizeHue(float h);
        static float RgbFromHue(float min, float max, float hue);

        float h_ = 0.0f;
        float s_ = 0.0f;
        float l_ = 0.0f;
    };
}
