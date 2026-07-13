// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Ogmo/OgmoJsonParser.hpp"

#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/Converters/OgmoTilemapDataConverter.hpp"
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"
#include "CNA/Extended/Tilemaps/TilemapFactory.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/IO/Path.hpp"
#include "System/Text/Json/JsonException.hpp"
#include "System/Text/Json/JsonSerializer.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace CNA::Extended::Tilemaps::Ogmo
{
    using Parsers::TilemapParseException;

    namespace
    {
        std::string GetDirectoryOrCurrent(const std::string& path)
        {
            const std::string directory = System::IO::Path::GetDirectoryName(path);
            if (directory.empty())
            {
                return std::filesystem::current_path().string();
            }
            return directory;
        }

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
    }

    OgmoJsonParser::OgmoJsonParser(
        const std::string& projectPath, const std::optional<std::string>& baseDirectory, const std::optional<Content::ExternalResourceResolver>& resourceResolver)
        : projectPath_(projectPath),
          baseDirectory_(baseDirectory.value_or(GetDirectoryOrCurrent(projectPath))),
          resourceResolver_(resourceResolver.value_or(Content::ExternalResourceResolver(&Content::OpenFile)))
    {
    }

    bool OgmoJsonParser::CanParse(const std::string& filePath) const
    {
        if (filePath.empty())
        {
            return false;
        }

        const std::string extension = System::IO::Path::GetExtension(filePath);
        return std::any_of(supportedExtensions_.begin(), supportedExtensions_.end(),
            [&extension](const std::string& supported)
            {
                if (extension.size() != supported.size())
                {
                    return false;
                }
                return std::equal(extension.begin(), extension.end(), supported.begin(),
                    [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
            });
    }

    Tilemap OgmoJsonParser::ParseFromFile(const std::string& path, GraphicsDevice& graphicsDevice)
    {
        if (path.empty())
        {
            throw std::invalid_argument("path must not be empty.");
        }

        const std::string fullPath = System::IO::Path::Combine(baseDirectory_, path);

        try
        {
            const Document::OgmoProject project = LoadProject(projectPath_);
            const Document::OgmoLevel level = LoadLevel(fullPath);

            const std::string projectDirectory = GetDirectoryOrCurrent(projectPath_);

            return ConvertLevel(level, project, graphicsDevice, projectDirectory);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("Failed to parse Ogmo level file: " + fullPath);
        }
    }

    Tilemap OgmoJsonParser::ParseFromStream(System::IO::Stream& stream, GraphicsDevice& graphicsDevice, const std::string& basePath)
    {
        try
        {
            Document::OgmoLevel level;
            try
            {
                level = System::Text::Json::JsonSerializer::Deserialize<Document::OgmoLevel>(ReadAllText(stream));
            }
            catch (const System::Text::Json::JsonException& ex)
            {
                throw TilemapParseException("Failed to parse Ogmo JSON data.", std::make_exception_ptr(ex));
            }

            const Document::OgmoProject project = LoadProject(projectPath_);

            const std::string resolvedBasePath = !basePath.empty() ? basePath : baseDirectory_;

            return ConvertLevel(level, project, graphicsDevice, resolvedBasePath);
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("An unexpected error occurred while parsing Ogmo data.");
        }
    }

    Document::OgmoProject OgmoJsonParser::LoadProject(const std::string& projectPath) const
    {
        try
        {
            const std::unique_ptr<System::IO::Stream> stream = OpenProjectStream(projectPath);
            return System::Text::Json::JsonSerializer::Deserialize<Document::OgmoProject>(ReadAllText(*stream));
        }
        catch (const System::Text::Json::JsonException&)
        {
            throw TilemapParseException("Failed to parse Ogmo project file: " + projectPath);
        }
    }

    Document::OgmoLevel OgmoJsonParser::LoadLevel(const std::string& levelPath) const
    {
        const std::unique_ptr<System::IO::Stream> stream = OpenLevelStream(levelPath);
        try
        {
            return System::Text::Json::JsonSerializer::Deserialize<Document::OgmoLevel>(ReadAllText(*stream));
        }
        catch (const System::Text::Json::JsonException&)
        {
            throw TilemapParseException("Failed to parse Ogmo level from file: " + levelPath);
        }
    }

    Tilemap OgmoJsonParser::ConvertLevel(
        const Document::OgmoLevel& level, const Document::OgmoProject& project, GraphicsDevice& graphicsDevice, const std::string& baseDirectory) const
    {
        TilemapData data = Converters::Convert(level, project, baseDirectory, resourceResolver_);
        return Build(data, graphicsDevice, baseDirectory, resourceResolver_);
    }

    std::unique_ptr<System::IO::Stream> OgmoJsonParser::OpenProjectStream(const std::string& projectPath) const
    {
        try
        {
            return resourceResolver_(projectPath);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("Failed to open Ogmo project file: " + projectPath);
        }
    }

    std::unique_ptr<System::IO::Stream> OgmoJsonParser::OpenLevelStream(const std::string& levelPath) const
    {
        try
        {
            return resourceResolver_(levelPath);
        }
        catch (const System::IO::FileNotFoundException&)
        {
            throw;
        }
        catch (const TilemapParseException&)
        {
            throw;
        }
        catch (const std::exception&)
        {
            throw TilemapParseException("Failed to open Ogmo level file: " + levelPath);
        }
    }
}
