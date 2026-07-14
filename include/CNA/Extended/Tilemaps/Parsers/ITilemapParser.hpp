// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Parsers/ITilemapParser.cs. `Tilemap ParseFromFile/
// ParseFromStream` return by value: `Tilemap` is move-only (it owns `TilemapTilesetCollection`/
// `TilemapLayerCollection`, themselves owning `unique_ptr` collections), but move-only return
// types work fine by value in C++ (guaranteed copy elision / move construction) -- no artificial
// `std::unique_ptr<Tilemap>` wrapping needed, and `Tilemap` isn't a polymorphic type that would
// need one for slicing-safety reasons either.
#pragma once

#include "CNA/Extended/Tilemaps/Tilemap.hpp"

#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace System::IO
{
    class Stream;
}

namespace CNA::Extended::Tilemaps::Parsers
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /** @brief Interface for tilemap format parsers (Tiled, LDtk, Ogmo). */
    class ITilemapParser
    {
    public:
        virtual ~ITilemapParser() = default;

        /** @brief Gets the file extensions supported by this parser (e.g. ".tmx", ".ldtk"). */
        [[nodiscard]] virtual const std::vector<std::string>& getSupportedExtensionsProperty() const = 0;

        /**
         * @brief Parses a tilemap from the file at @p path.
         * @throws TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] virtual Tilemap ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice) = 0;

        /**
         * @brief Parses a tilemap from @p stream.
         * @param stream The stream containing the map data.
         * @param graphicsDevice The graphics device used to create textures for referenced tileset images.
         * @param basePath The base path for resolving relative file references.
         * @throws TilemapParseException the stream cannot be parsed.
         */
        [[nodiscard]] virtual Tilemap ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath) = 0;
    };
}
