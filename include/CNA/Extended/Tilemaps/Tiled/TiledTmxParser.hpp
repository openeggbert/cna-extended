// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tiled/TiledTmxParser.cs. Implements
// Parsers::ITilemapParser (already ported).
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"
#include "CNA/Extended/Tilemaps/Parsers/ITilemapParser.hpp"
#include "CNA/Extended/Tilemaps/Tiled/Document/TiledDocument.hpp"

#include <memory>
#include <optional>
#include <string>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Tilemaps::Tiled
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /**
     * @brief Parses TMX XML text into the raw Document::TiledMapXml object graph, with no
     * texture loading and no GraphicsDevice needed (that only happens in TilemapFactory::Build,
     * downstream of Converters::Convert). Exposed publicly (not just used internally by
     * ParseFromFile/ParseFromStream) so callers -- including tests -- can exercise the XML
     * parsing and Document-to-TilemapData conversion stages without a live GraphicsDevice,
     * matching this project's established "no headless GraphicsDevice test infra" gap: this is
     * the boundary that stays fully headless-testable.
     * @throws Parsers::TilemapParseException @p xmlText is not valid TMX XML.
     */
    [[nodiscard]] std::unique_ptr<Document::TiledMapXml> ParseMapXml(const std::string& xmlText);

    /** @brief Parser for Tiled TMX (Tile Map XML) files. */
    class TiledTmxParser : public Parsers::ITilemapParser
    {
    public:
        /**
         * @brief Creates a parser.
         * @param baseDirectory Optional base directory for resolving relative file paths in ParseFromFile. If empty, paths are resolved from the file's own location.
         * @param resourceResolver Optional resolver used to open external resources referenced by the map. If unset, resources are opened from the local file system.
         */
        explicit TiledTmxParser(
            const std::optional<std::string>& baseDirectory = std::nullopt,
            const std::optional<Content::ExternalResourceResolver>& resourceResolver = std::nullopt);

        [[nodiscard]] const std::vector<std::string>& getSupportedExtensionsProperty() const override { return supportedExtensions_; }

        /**
         * @brief Parses a Tiled TMX file from disk.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         * @throws System::IO::FileNotFoundException the file does not exist.
         */
        [[nodiscard]] Tilemap ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice) override;

        /**
         * @brief Parses a Tiled TMX file from a stream.
         * @param basePath The base path for resolving relative file references. If empty, uses the base directory from the constructor, or the current directory if neither is set.
         * @throws Parsers::TilemapParseException the stream cannot be parsed.
         */
        [[nodiscard]] Tilemap ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath) override;

    private:
        std::optional<std::string> baseDirectory_;
        Content::ExternalResourceResolver resourceResolver_;
        std::vector<std::string> supportedExtensions_{".tmx"};
    };
}
