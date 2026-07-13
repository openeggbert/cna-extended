// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tiled/Converters/TiledTilemapDataConverter.cs.
// Upstream's `internal static class` -> free function(s) in this namespace.
//
// Includes the tile-data decoding logic (base64/CSV/plain-XML, gzip/zlib decompression)
// inlined directly here, matching upstream's own actual structure: the standalone
// `TiledDataDecoder.cs`/`TiledPropertyConverter.cs` classes are dead code upstream (confirmed
// via grep: zero call sites anywhere in either source/ or tests/) -- this converter has its own
// complete, independent copy of the same decode logic, which is what's actually exercised. Not
// porting the two dead standalone classes avoids literal duplicate decoding logic in this port
// for code upstream itself never calls; add them later if a real caller needs them.
#pragma once

#include "CNA/Extended/Tilemaps/Tiled/Document/TiledDocument.hpp"
#include "CNA/Extended/Tilemaps/TilemapData.hpp"

namespace CNA::Extended::Tilemaps::Tiled::Converters
{
    /**
     * @brief Converts a parsed TiledMapXml document to the format-agnostic TilemapData intermediate representation.
     * @throws Parsers::TilemapParseException the map cannot be converted (e.g. unsupported tile-data encoding/compression).
     */
    [[nodiscard]] TilemapData Convert(const Document::TiledMapXml& map);
}
