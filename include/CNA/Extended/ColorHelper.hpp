// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ColorHelper.cs. Upstream builds its name->Color lookup table
// via reflection (typeof(Color).GetRuntimeProperties(), filtered to Color-typed properties) --
// C++ has no runtime reflection, so the table is a hand-written (script-generated, cross-checked
// against CNA's Color.hpp) static list of all 141 of CNA's named Color constants instead. Lookup
// stays case-insensitive (matching upstream's StringComparer.OrdinalIgnoreCase) by lower-casing
// both the stored keys and the lookup input.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <cstdint>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Color;

    /** @brief Provides utility methods for working with Color values. */
    class ColorHelper
    {
    public:
        ColorHelper() = delete;

        /**
         * @brief Converts a hexadecimal color string to a Color value. Supports 3 (RGB
         * shorthand), 4 (RGBA shorthand), 6 (RGB), and 8 (RGBA) character formats, with an
         * optional '#' prefix.
         * @param value The hexadecimal color string to convert.
         * @return The parsed Color, or Color::Transparent if @p value is empty.
         * @throws std::invalid_argument if the length (excluding a '#' prefix) is not 3, 4, 6,
         * or 8.
         */
        [[nodiscard]] static Color FromHex(const std::string& value);

        /**
         * @brief Creates a Color value from the name of a predefined color (case-insensitive).
         * @param name The name of the predefined color.
         * @return The Color value for @p name.
         * @throws std::invalid_argument if @p name is not a valid color name.
         */
        [[nodiscard]] static Color FromName(const std::string& name);

        /**
         * @brief Creates a Color value from a packed value in the ABGR format.
         * @param abgr The packed color value in ABGR format.
         * @return The Color value created.
         */
        [[nodiscard]] static Color FromAbgr(std::uint32_t abgr);
    };
}
