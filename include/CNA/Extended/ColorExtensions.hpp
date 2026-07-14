// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ColorExtensions.cs. C# extension methods have no C++
// equivalent syntax; ported as a free function in this namespace, matching the convention used
// throughout this project for extension methods.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Color;

    /**
     * @brief Converts a Color to its hexadecimal string representation in RGBA format.
     * @param color The Color to convert.
     * @return A hexadecimal string representation of the color in the format `#RRGGBBAA`.
     */
    [[nodiscard]] std::string ToHex(const Color& color);
}
