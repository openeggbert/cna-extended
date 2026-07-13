// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Ogmo/OgmoColorParser.cs. Upstream's `internal static
// class` -> free function in this namespace.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <string>

namespace CNA::Extended::Tilemaps::Ogmo
{
    using Microsoft::Xna::Framework::Color;

    /**
     * @brief Parses an Ogmo "#RRGGBBAA" color string.
     * @return The parsed color, or Color::White if @p colorString is empty, doesn't start with
     * '#', or isn't exactly 8 hex digits after the '#' -- matching upstream's own fallback
     * behavior exactly (not an exception-throwing parse).
     */
    [[nodiscard]] Color ParseColor(const std::string& colorString);
}
