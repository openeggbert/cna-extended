// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapFactory.cs: builds the runtime `Tilemap`
// object graph from a `TilemapData` intermediate representation (produced by format parsers),
// loading textures via the provided `GraphicsDevice`/`ExternalResourceResolver`. Upstream's
// `static class` -> free functions, matching this project's established convention.
//
// NOTE: `Build` takes @p data by non-const reference and genuinely mutates it (group-layer
// child names get a "Group/SubGroup/" prefix written back into the DTO's own `Name` field
// during flattening) -- this is upstream's own real behavior (`layerData.Name = pathPrefix +
// "/" + layerData.Name;`), not a translation artifact; preserved as-is since `data` is
// intermediate, single-use parser output, not something callers are expected to reuse
// afterward.
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"
#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapData.hpp"

#include <string>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /**
     * @brief Builds a Tilemap from @p data, resolving relative texture paths against @p baseDirectory via
     * CNA::Extended::Content::OpenFile.
     * @throws Parsers::TilemapParseException a texture cannot be loaded.
     */
    [[nodiscard]] Tilemap Build(TilemapData& data, GraphicsDevice& graphicsDevice, const std::string& baseDirectory);

    /**
     * @brief Builds a Tilemap from @p data, resolving relative texture paths against @p baseDirectory via @p resourceResolver.
     * @throws Parsers::TilemapParseException a texture cannot be loaded.
     */
    [[nodiscard]] Tilemap Build(TilemapData& data, GraphicsDevice& graphicsDevice, const std::string& baseDirectory,
        const Content::ExternalResourceResolver& resourceResolver);
}
