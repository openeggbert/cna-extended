// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Ogmo/Converters/OgmoTilemapDataConverter.cs. Upstream's
// `internal static class` -> free function(s) in this namespace, matching Tiled::Converters::Convert
// (see that file for the established "document model -> TilemapData" architecture this follows).
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/Document/OgmoDocument.hpp"
#include "CNA/Extended/Tilemaps/TilemapData.hpp"

#include <string>

namespace CNA::Extended::Tilemaps::Ogmo::Converters
{
    /** @brief Converts a parsed Ogmo level+project pair to the format-agnostic TilemapData intermediate representation, resolving tileset image dimensions from the current directory via CNA::Extended::Content::OpenFile. */
    [[nodiscard]] TilemapData Convert(const Document::OgmoLevel& level, const Document::OgmoProject& project);

    /**
     * @brief Converts a parsed Ogmo level+project pair to the format-agnostic TilemapData intermediate representation.
     * @param level The parsed Ogmo level document.
     * @param project The parsed Ogmo project document referenced by @p level.
     * @param baseDirectory Directory relative tileset image paths are resolved against.
     * @param resourceResolver Used to open tileset images to read their pixel dimensions (needed to compute tile-grid column/row counts).
     */
    [[nodiscard]] TilemapData Convert(const Document::OgmoLevel& level, const Document::OgmoProject& project,
        const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver);
}
