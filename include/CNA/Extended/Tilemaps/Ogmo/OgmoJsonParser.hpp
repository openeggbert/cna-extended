// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Ogmo/OgmoJsonParser.cs. Implements
// Parsers::ITilemapParser (already ported). Unlike TiledTmxParser's `baseDirectory_`
// (`std::optional<std::string>`, resolved lazily per parse call), upstream resolves
// `_baseDirectory` once, in the constructor, to a concrete (never-empty) value -- ported
// faithfully as a plain (non-optional) `std::string` member, matching that real difference in
// upstream's own two parsers' designs rather than forcing both into an identical shape.
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/Document/OgmoDocument.hpp"
#include "CNA/Extended/Tilemaps/Parsers/ITilemapParser.hpp"

#include <optional>
#include <string>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Tilemaps::Ogmo
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /** @brief Parser for Ogmo Editor 3 JSON tilemap files. */
    class OgmoJsonParser : public Parsers::ITilemapParser
    {
    public:
        /**
         * @brief Creates a parser.
         * @param projectPath The path to the Ogmo project file (.ogmo).
         * @param baseDirectory Optional base directory for resolving relative level file paths. If unset, the project file's own directory is used.
         * @param resourceResolver Optional resolver used to open external resources referenced by the project. If unset, resources are opened from the local file system.
         */
        explicit OgmoJsonParser(const std::string& projectPath, const std::optional<std::string>& baseDirectory = std::nullopt,
            const std::optional<Content::ExternalResourceResolver>& resourceResolver = std::nullopt);

        [[nodiscard]] const std::vector<std::string>& getSupportedExtensionsProperty() const override { return supportedExtensions_; }

        /** @brief Determines whether @p filePath's extension is supported by this parser. */
        [[nodiscard]] bool CanParse(const std::string& filePath) const;

        /**
         * @brief Parses a tilemap from the specified Ogmo level file.
         * @param path The path to the Ogmo level file (.json). Can be absolute or relative to the base directory specified in the constructor.
         * @param graphicsDevice The graphics device used to create textures for referenced tileset images.
         * @throws std::invalid_argument @p path is empty.
         * @throws System::IO::FileNotFoundException the specified level file does not exist.
         * @throws Parsers::TilemapParseException an error occurred while parsing the file.
         */
        [[nodiscard]] Tilemap ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice) override;

        /**
         * @brief Parses a tilemap from a stream containing Ogmo level data.
         * @param stream The stream containing the Ogmo level JSON data.
         * @param graphicsDevice The graphics device used to create textures for referenced tileset images.
         * @param basePath Optional base path for resolving relative file references. If empty, uses the project file's directory.
         * @throws Parsers::TilemapParseException an error occurred while parsing the stream.
         */
        [[nodiscard]] Tilemap ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath) override;

    private:
        [[nodiscard]] Document::OgmoProject LoadProject(const std::string& projectPath) const;
        [[nodiscard]] Document::OgmoLevel LoadLevel(const std::string& levelPath) const;
        [[nodiscard]] Tilemap ConvertLevel(
            const Document::OgmoLevel& level, const Document::OgmoProject& project, GraphicsDevice& graphicsDevice, const std::string& baseDirectory) const;
        [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenProjectStream(const std::string& projectPath) const;
        [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenLevelStream(const std::string& levelPath) const;

        std::string projectPath_;
        std::string baseDirectory_;
        Content::ExternalResourceResolver resourceResolver_;
        std::vector<std::string> supportedExtensions_{".json"};
    };
}
