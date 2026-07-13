// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tiled/TiledColorParser.cs. Upstream's `internal
// static class` -> free functions in this namespace, matching this project's established
// convention. `Color? Parse(string)` -> `std::optional<Color> Parse(const std::string&)`.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"

#include <optional>
#include <string>

namespace CNA::Extended::Tilemaps::Tiled
{
    using Microsoft::Xna::Framework::Color;

    /**
     * @brief Parses a Tiled color string (#RRGGBB or #AARRGGBB), or std::nullopt for empty/whitespace input.
     * @throws Parsers::TilemapParseException @p colorString is non-empty but not a valid #RRGGBB/#AARRGGBB color.
     */
    [[nodiscard]] std::optional<Color> Parse(const std::string& colorString);

    /** @brief Non-throwing variant of Parse(); returns false and leaves @p color unset if parsing fails. */
    [[nodiscard]] bool TryParse(const std::string& colorString, std::optional<Color>& color);
}
