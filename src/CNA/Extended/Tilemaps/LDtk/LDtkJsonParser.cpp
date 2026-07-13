// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/LDtk/LDtkJsonParser.hpp"

#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"
#include "CNA/Extended/Tilemaps/LDtk/Converters/LDtkTilemapDataConverter.hpp"
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"
#include "CNA/Extended/Tilemaps/TilemapFactory.hpp"
#include "CNA/Extended/Tilemaps/TilemapPropertyValue.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/Path.hpp"

#include <filesystem>
#include <stdexcept>

namespace CNA::Extended::Tilemaps::LDtk
{
    using Parsers::TilemapParseException;

    namespace
    {
        std::string ReadAllText(System::IO::Stream& stream)
        {
            const SharpRuntime::intcs length = stream.getLengthProperty();
            std::vector<SharpRuntime::bytecs> buffer(static_cast<std::size_t>(length));

            SharpRuntime::intcs totalRead = 0;
            while (totalRead < length)
            {
                const SharpRuntime::intcs read = stream.Read(buffer.data(), totalRead, length - totalRead);
                if (read == 0)
                {
                    break;
                }
                totalRead += read;
            }

            return std::string(buffer.begin(), buffer.begin() + totalRead);
        }

        Document::LDtkProject ParseProjectJson(const std::string& jsonText)
        {
            try
            {
                return nlohmann::ordered_json::parse(jsonText).get<Document::LDtkProject>();
            }
            catch (const std::exception& ex)
            {
                throw TilemapParseException(std::string("Failed to parse LDtk JSON data: ") + ex.what());
            }
        }
    }

    LDtkJsonParser::LDtkJsonParser(
        const std::optional<std::string>& baseDirectory, const std::optional<Content::ExternalResourceResolver>& resourceResolver)
        : baseDirectory_(baseDirectory), resourceResolver_(resourceResolver.value_or(Content::ExternalResourceResolver(&Content::OpenFile)))
    {
    }

    std::unique_ptr<System::IO::Stream> LDtkJsonParser::OpenProjectStream(const std::string& path) const
    {
        try
        {
            return resourceResolver_(path);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to open LDtk file: " + path + ": " + ex.what());
        }
    }

    Document::LDtkProject LDtkJsonParser::LoadProject(const std::string& path) const
    {
        const std::unique_ptr<System::IO::Stream> stream = OpenProjectStream(path);
        return ParseProjectJson(ReadAllText(*stream));
    }

    std::optional<Document::LDtkLevel> LDtkJsonParser::LoadExternalLevel(const Document::LDtkLevel& level, const std::string& levelPath) const
    {
        try
        {
            const std::unique_ptr<System::IO::Stream> stream = resourceResolver_(levelPath);
            return nlohmann::ordered_json::parse(ReadAllText(*stream)).get<Document::LDtkLevel>();
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Level '" + level.Identifier + "' references external file '" + level.ExternalRelPath +
                "' which could not be opened. Expected at: " + levelPath + ": " + ex.what());
        }
    }

    Tilemap LDtkJsonParser::ConvertLevel(
        Document::LDtkLevel level, const Document::LDtkProject& project, GraphicsDevice& graphicsDevice, const std::string& projectDirectory) const
    {
        const std::string baseDirectory = projectDirectory.empty() ? std::filesystem::current_path().string() : projectDirectory;

        if (!level.ExternalRelPath.empty())
        {
            const std::string levelPath = System::IO::Path::Combine(baseDirectory, level.ExternalRelPath);
            std::optional<Document::LDtkLevel> externalLevel = LoadExternalLevel(level, levelPath);
            if (externalLevel.has_value())
            {
                level = std::move(*externalLevel);
            }
        }

        TilemapData data = Converters::Convert(level, project);
        return Build(data, graphicsDevice, baseDirectory, resourceResolver_);
    }

    bool LDtkJsonParser::CanParse(const std::string& filePath) const
    {
        if (filePath.empty())
        {
            return false;
        }

        const std::string extension = System::IO::Path::GetExtension(filePath);
        return std::any_of(supportedExtensions_.begin(), supportedExtensions_.end(), [&extension](const std::string& supported)
            {
                return extension.size() == supported.size() &&
                    std::equal(extension.begin(), extension.end(), supported.begin(),
                        [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
            });
    }

    Tilemap LDtkJsonParser::ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice)
    {
        if (path.empty())
        {
            throw std::invalid_argument("path must not be empty.");
        }

        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const std::unique_ptr<System::IO::Stream> stream = OpenProjectStream(fullPath);
            const std::string directory = System::IO::Path::GetDirectoryName(fullPath);
            return ParseFromStream(*stream, graphicsDevice, directory);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to parse LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    Tilemap LDtkJsonParser::ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath)
    {
        try
        {
            const Document::LDtkProject project = ParseProjectJson(ReadAllText(stream));

            if (project.Levels.empty())
            {
                throw TilemapParseException("LDtk project contains no levels.");
            }

            const std::string resolvedBasePath =
                !basePath.empty() ? basePath : (baseDirectory_.has_value() ? *baseDirectory_ : std::filesystem::current_path().string());

            const Document::LDtkLevel& firstLevel = project.Levels.front();
            return ConvertLevel(firstLevel, project, graphicsDevice, resolvedBasePath);
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException(std::string("An unexpected error occurred while parsing LDtk data: ") + ex.what());
        }
    }

    Tilemap LDtkJsonParser::ParseLevel(const std::string& path, const std::string& levelIdentifier, GraphicsDevice& graphicsDevice)
    {
        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const std::string projectDirectory = System::IO::Path::GetDirectoryName(fullPath);
            const Document::LDtkProject project = LoadProject(fullPath);

            const auto it = std::find_if(project.Levels.begin(), project.Levels.end(),
                [&levelIdentifier](const Document::LDtkLevel& l) { return l.Identifier == levelIdentifier; });
            if (it == project.Levels.end())
            {
                throw std::invalid_argument("Level '" + levelIdentifier + "' not found in project.");
            }

            return ConvertLevel(*it, project, graphicsDevice, projectDirectory);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::invalid_argument&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to parse level '" + levelIdentifier + "' from LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    std::vector<Tilemap> LDtkJsonParser::ParseAllLevels(const std::string& path, GraphicsDevice& graphicsDevice)
    {
        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const std::string projectDirectory = System::IO::Path::GetDirectoryName(fullPath);
            const Document::LDtkProject project = LoadProject(fullPath);

            std::vector<Tilemap> tilemaps;
            tilemaps.reserve(project.Levels.size());
            for (const Document::LDtkLevel& level : project.Levels)
            {
                tilemaps.push_back(ConvertLevel(level, project, graphicsDevice, projectDirectory));
            }

            return tilemaps;
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to parse LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    std::vector<Tilemap> LDtkJsonParser::ParseWorld(const std::string& path, const std::string& worldIdentifier, GraphicsDevice& graphicsDevice)
    {
        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const std::string projectDirectory = System::IO::Path::GetDirectoryName(fullPath);
            const Document::LDtkProject project = LoadProject(fullPath);

            if (!project.Worlds.empty())
            {
                const auto it = std::find_if(project.Worlds.begin(), project.Worlds.end(),
                    [&worldIdentifier](const Document::LDtkWorld& w) { return w.Identifier == worldIdentifier; });
                if (it == project.Worlds.end())
                {
                    throw std::invalid_argument("World '" + worldIdentifier + "' not found in project.");
                }

                std::vector<Tilemap> tilemaps;
                tilemaps.reserve(it->Levels.size());
                for (const Document::LDtkLevel& level : it->Levels)
                {
                    Tilemap tilemap = ConvertLevel(level, project, graphicsDevice, projectDirectory);
                    tilemap.getPropertiesProperty().SetString("LDtk_WorldIid", it->Iid);
                    tilemap.getPropertiesProperty().SetString("LDtk_WorldIdentifier", it->Identifier);
                    tilemaps.push_back(std::move(tilemap));
                }

                return tilemaps;
            }

            throw std::logic_error(
                "This LDtk project does not have multi-worlds enabled. "
                "Use ParseAllLevels() instead, or enable multi-worlds in LDtk project settings.");
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::invalid_argument&)
        {
            throw;
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const std::logic_error&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to parse world '" + worldIdentifier + "' from LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    std::vector<std::string> LDtkJsonParser::GetWorldIdentifiers(const std::string& path)
    {
        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const Document::LDtkProject project = LoadProject(fullPath);

            std::vector<std::string> identifiers;
            if (!project.Worlds.empty())
            {
                for (const Document::LDtkWorld& world : project.Worlds)
                {
                    if (!world.Identifier.empty())
                    {
                        identifiers.push_back(world.Identifier);
                    }
                }
            }

            return identifiers;
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to read world list from LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    std::map<std::string, std::vector<Document::LDtkTocInstanceData>> LDtkJsonParser::GetTableOfContents(const std::string& path)
    {
        const std::string fullPath = baseDirectory_.has_value() ? System::IO::Path::Combine(*baseDirectory_, path) : path;

        try
        {
            const Document::LDtkProject project = LoadProject(fullPath);

            std::map<std::string, std::vector<Document::LDtkTocInstanceData>> toc;
            for (const Document::LDtkTableOfContentEntry& entry : project.Toc)
            {
                if (!entry.Identifier.empty())
                {
                    toc[entry.Identifier] = entry.InstancesData;
                }
            }

            return toc;
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception& ex)
        {
            throw TilemapParseException("Failed to read table of contents from LDtk file: " + fullPath + ": " + ex.what());
        }
    }

    std::optional<Point> LDtkJsonParser::GetWorldPosition(const Tilemap& tilemap)
    {
        TilemapPropertyValue worldXValue;
        TilemapPropertyValue worldYValue;
        const bool hasWorldX = tilemap.getPropertiesProperty().TryGetValue("LDtk_WorldX", worldXValue);
        const bool hasWorldY = tilemap.getPropertiesProperty().TryGetValue("LDtk_WorldY", worldYValue);

        if (hasWorldX && hasWorldY)
        {
            return Point(worldXValue.AsInt(), worldYValue.AsInt());
        }

        return std::nullopt;
    }

    const Tilemap* LDtkJsonParser::FindLevelByIid(const std::vector<Tilemap>& tilemaps, const std::string& worldIid)
    {
        if (worldIid.empty())
        {
            return nullptr;
        }

        for (const Tilemap& tilemap : tilemaps)
        {
            TilemapPropertyValue iidValue;
            if (tilemap.getPropertiesProperty().TryGetValue("LDtk_Iid", iidValue) && iidValue.AsString() == worldIid)
            {
                return &tilemap;
            }
        }

        return nullptr;
    }
}
