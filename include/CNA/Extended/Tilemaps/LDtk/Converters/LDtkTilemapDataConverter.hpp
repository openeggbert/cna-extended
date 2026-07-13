// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/LDtk/Converters/LDtkTilemapDataConverter.cs.
// Upstream's `internal static class` -> free function(s) in this namespace, matching this
// project's established convention and `Tiled::Converters::Convert`'s established shape.
//
// Unlike Tiled (one map -> one TilemapData), LDtk projects contain multiple levels, each
// independently convertible to its own TilemapData -- `Convert` takes a single already-selected
// `LDtkLevel` (matching upstream's own `Convert(LDtkLevel level, LDtkProject project)` signature);
// iterating a whole project's levels is `LDtkJsonParser`'s job (already established in that type),
// not this converter's.
#pragma once

#include "CNA/Extended/Tilemaps/LDtk/Document/LDtkDocument.hpp"
#include "CNA/Extended/Tilemaps/TilemapData.hpp"

namespace CNA::Extended::Tilemaps::LDtk::Converters
{
    /**
     * @brief Converts a single parsed LDtk level (plus its owning project, for shared tileset
     * definitions) to the format-agnostic TilemapData intermediate representation.
     * @throws Parsers::TilemapParseException a field instance cannot be converted.
     */
    [[nodiscard]] TilemapData Convert(const Document::LDtkLevel& level, const Document::LDtkProject& project);
}
