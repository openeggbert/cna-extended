// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/LDtk/LDtkJsonParser.cs. Implements
// Parsers::ITilemapParser (already ported), plus LDtk-specific convenience methods beyond the
// interface (ParseLevel/ParseAllLevels/ParseWorld/GetWorldIdentifiers/GetTableOfContents/
// GetWorldPosition/FindLevelByIid) that exist on upstream's own public API surface, not just the
// interface it implements -- ported here for the same "port faithfully" reason.
#pragma once

#include "CNA/Extended/Content/ExternalResourceResolver.hpp"
#include "CNA/Extended/Tilemaps/LDtk/Document/LDtkDocument.hpp"
#include "CNA/Extended/Tilemaps/Parsers/ITilemapParser.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::Tilemaps::LDtk
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    /** @brief Parser for LDtk JSON tilemap project files. */
    class LDtkJsonParser : public Parsers::ITilemapParser
    {
    public:
        /**
         * @brief Creates a parser.
         * @param baseDirectory Optional base directory for resolving relative file paths in ParseFromFile. If unset, paths are resolved from the file's own location.
         * @param resourceResolver Optional resolver used to open external resources referenced by the project. If unset, resources are opened from the local file system.
         */
        explicit LDtkJsonParser(const std::optional<std::string>& baseDirectory = std::nullopt,
            const std::optional<Content::ExternalResourceResolver>& resourceResolver = std::nullopt);

        [[nodiscard]] const std::vector<std::string>& getSupportedExtensionsProperty() const override { return supportedExtensions_; }

        /** @brief Determines whether @p filePath has a supported LDtk extension (case-insensitive). Not part of ITilemapParser -- LDtk-specific, unlike TiledTmxParser, matching upstream's own asymmetry. */
        [[nodiscard]] bool CanParse(const std::string& filePath) const;

        /**
         * @brief Parses a tilemap from the LDtk project file at @p path, representing the first level in the project.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         * @throws System::IO::FileNotFoundException the file does not exist.
         */
        [[nodiscard]] Tilemap ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice) override;

        /**
         * @brief Parses a tilemap from an LDtk project stream, representing the first level in the project.
         * @param basePath The base path for resolving relative file references. If empty, uses the base directory from the constructor, or the current directory if neither is set.
         * @throws Parsers::TilemapParseException the stream cannot be parsed.
         */
        [[nodiscard]] Tilemap ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath) override;

        /**
         * @brief Parses a specific level, by identifier, from an LDtk project file.
         * @throws std::invalid_argument the specified level was not found in the project.
         * @throws System::IO::FileNotFoundException the file does not exist.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] Tilemap ParseLevel(const std::string& path, const std::string& levelIdentifier, GraphicsDevice& graphicsDevice);

        /**
         * @brief Parses every level from an LDtk project file.
         * @throws System::IO::FileNotFoundException the file does not exist.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] std::vector<Tilemap> ParseAllLevels(const std::string& path, GraphicsDevice& graphicsDevice);

        /**
         * @brief Parses every level from a specific world in a multi-world LDtk project.
         * @throws std::invalid_argument the specified world was not found in the project.
         * @throws std::logic_error this project does not have multi-worlds enabled.
         * @throws System::IO::FileNotFoundException the file does not exist.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] std::vector<Tilemap> ParseWorld(const std::string& path, const std::string& worldIdentifier, GraphicsDevice& graphicsDevice);

        /**
         * @brief Gets the list of world identifiers from an LDtk project file, or an empty list if multi-worlds is not enabled.
         * @throws System::IO::FileNotFoundException the file does not exist.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] std::vector<std::string> GetWorldIdentifiers(const std::string& path);

        /**
         * @brief Gets the table of contents from an LDtk project file, mapping entity identifiers to their instance data.
         * @throws System::IO::FileNotFoundException the file does not exist.
         * @throws Parsers::TilemapParseException the file cannot be parsed.
         */
        [[nodiscard]] std::map<std::string, std::vector<Document::LDtkTocInstanceData>> GetTableOfContents(const std::string& path);

        /** @brief Gets the world position of @p tilemap from its "LDtk_WorldX"/"LDtk_WorldY" properties, or nullopt if not available. */
        [[nodiscard]] static std::optional<Point> GetWorldPosition(const Tilemap& tilemap);

        /** @brief Finds the tilemap in @p tilemaps whose "LDtk_Iid" property matches @p worldIid, or nullptr if not found. */
        [[nodiscard]] static const Tilemap* FindLevelByIid(const std::vector<Tilemap>& tilemaps, const std::string& worldIid);

    private:
        [[nodiscard]] Document::LDtkProject LoadProject(const std::string& path) const;
        [[nodiscard]] Tilemap ConvertLevel(
            Document::LDtkLevel level, const Document::LDtkProject& project, GraphicsDevice& graphicsDevice, const std::string& projectDirectory) const;
        [[nodiscard]] std::unique_ptr<System::IO::Stream> OpenProjectStream(const std::string& path) const;
        [[nodiscard]] std::optional<Document::LDtkLevel> LoadExternalLevel(const Document::LDtkLevel& level, const std::string& levelPath) const;

        std::optional<std::string> baseDirectory_;
        Content::ExternalResourceResolver resourceResolver_;
        std::vector<std::string> supportedExtensions_{".ldtk"};
    };
}
