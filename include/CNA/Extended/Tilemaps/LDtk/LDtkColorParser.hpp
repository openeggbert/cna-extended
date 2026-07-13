// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/LDtk/LDtkColorParser.cs. Upstream's `internal static
// class` -> free functions in this namespace, matching this project's established convention.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <string>

namespace CNA::Extended::Tilemaps::LDtk
{
    using Microsoft::Xna::Framework::Color;

    /** @brief Parses an LDtk "#RRGGBB" hex color string. Returns Color::Transparent for null/empty/malformed input, matching upstream. */
    [[nodiscard]] Color ParseColor(const std::string& colorString);

    /** @brief Tries to parse @p colorString, matching upstream's own (slightly odd) "success" heuristic: false whenever the parsed result equals Transparent, unless the input was literally "#000000". */
    [[nodiscard]] bool TryParseColor(const std::string& colorString, Color& color);
}
