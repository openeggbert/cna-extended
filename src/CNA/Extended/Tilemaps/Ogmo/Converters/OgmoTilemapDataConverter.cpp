// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Preserves a genuine upstream quirk, not "fixed": `Convert`'s Name computation is
// `Path.GetFileNameWithoutExtension(level.OgmoVersion)`, applying a filename-extension strip to
// what is actually a version string (e.g. "3.4.0" -> "3") rather than a file path -- almost
// certainly an upstream copy-paste bug (probably meant to strip the level *file path*'s
// extension), but ported exactly as upstream behaves, per this port's faithfulness mandate.
#include "CNA/Extended/Tilemaps/Ogmo/Converters/OgmoTilemapDataConverter.hpp"

#include "CNA/Extended/Content/ExternalResourceResolvers.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/OgmoColorParser.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "System/Convert.hpp"
#include "System/IO/Path.hpp"
#include "System/IO/Stream.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>

namespace CNA::Extended::Tilemaps::Ogmo::Converters
{
    using Document::OgmoDecal;
    using Document::OgmoDecalLayerData;
    using Document::OgmoEntity;
    using Document::OgmoEntityLayerData;
    using Document::OgmoGridLayerData;
    using Document::OgmoLayerData;
    using Document::OgmoLayerTemplate;
    using Document::OgmoLevel;
    using Document::OgmoProject;
    using Document::OgmoTileLayerData;
    using Microsoft::Xna::Framework::Point;

    namespace
    {
        struct TilesetGidInfo
        {
            int FirstGid = 0;
            int Columns = 0;
        };

        void AddStringProperty(std::vector<TilemapPropertyData>& target, const std::string& key, const std::string& value)
        {
            TilemapPropertyData property;
            property.Key = key;
            property.Type = TilemapPropertyType::String;
            property.StringValue = value;
            target.push_back(std::move(property));
        }

        void AddIntProperty(std::vector<TilemapPropertyData>& target, const std::string& key, int value)
        {
            TilemapPropertyData property;
            property.Key = key;
            property.Type = TilemapPropertyType::Int;
            property.IntValue = value;
            target.push_back(std::move(property));
        }

        void AddFloatProperty(std::vector<TilemapPropertyData>& target, const std::string& key, float value)
        {
            TilemapPropertyData property;
            property.Key = key;
            property.Type = TilemapPropertyType::Float;
            property.FloatValue = value;
            target.push_back(std::move(property));
        }

        void AddBoolProperty(std::vector<TilemapPropertyData>& target, const std::string& key, bool value)
        {
            TilemapPropertyData property;
            property.Key = key;
            property.Type = TilemapPropertyType::Bool;
            property.BoolValue = value;
            target.push_back(std::move(property));
        }

        // Mirrors upstream ConvertJsonElement: inspect the raw JSON node's kind directly (the C++
        // analogue of C#'s JsonElement.ValueKind switch -- see OgmoDocument.hpp's header comment
        // for why storing raw nlohmann::ordered_json for "object"-typed fields is the more direct
        // translation here).
        void ConvertJsonValue(std::vector<TilemapPropertyData>& properties, const std::string& key, const nlohmann::ordered_json& element)
        {
            if (element.is_boolean())
            {
                AddBoolProperty(properties, key, element.get<bool>());
            }
            else if (element.is_number_integer())
            {
                AddIntProperty(properties, key, element.get<int>());
            }
            else if (element.is_number_float())
            {
                AddFloatProperty(properties, key, element.get<float>());
            }
            else if (element.is_string())
            {
                AddStringProperty(properties, key, element.get<std::string>());
            }
            else
            {
                AddStringProperty(properties, key, element.dump());
            }
        }

        void ConvertCustomValue(std::vector<TilemapPropertyData>& properties, const std::string& key, const nlohmann::ordered_json& value)
        {
            if (value.is_null())
            {
                AddStringProperty(properties, key, "");
            }
            else
            {
                ConvertJsonValue(properties, key, value);
            }
        }

        const OgmoLayerTemplate* FindTemplate(const OgmoProject& project, const std::string& layerName)
        {
            if (layerName.empty())
            {
                return nullptr;
            }

            for (const OgmoLayerTemplate& candidate : project.Layers)
            {
                if (candidate.Name == layerName)
                {
                    return &candidate;
                }
            }

            return nullptr;
        }

        int CalculateCount(int imageSize, int tileSize, int separation)
        {
            if (tileSize <= 0 || imageSize <= 0)
            {
                return 0;
            }

            const int effectiveSize = tileSize + separation;
            return (imageSize + separation) / effectiveSize;
        }

        // PNG IHDR chunk: bytes 0-7 = signature, 8-11 = IHDR length, 12-15 = IHDR type,
        // 16-19 = width (big-endian), 20-23 = height (big-endian).
        Point ReadPngDimensionsFromBytes(const std::vector<SharpRuntime::bytecs>& header)
        {
            if (header.size() < 24)
            {
                return Point::Zero;
            }

            if (header[0] != 0x89 || header[1] != 0x50 || header[2] != 0x4E || header[3] != 0x47)
            {
                return Point::Zero;
            }

            const int width = (static_cast<int>(header[16]) << 24) | (static_cast<int>(header[17]) << 16) |
                (static_cast<int>(header[18]) << 8) | static_cast<int>(header[19]);
            const int height = (static_cast<int>(header[20]) << 24) | (static_cast<int>(header[21]) << 16) |
                (static_cast<int>(header[22]) << 8) | static_cast<int>(header[23]);
            return Point(width, height);
        }

        Point ReadDimensionsFromDataUri(const std::string& dataUri)
        {
            try
            {
                const std::size_t commaIndex = dataUri.find(',');
                if (commaIndex == std::string::npos)
                {
                    return Point::Zero;
                }

                const std::vector<SharpRuntime::bytecs> bytes = System::Convert::FromBase64String(dataUri.substr(commaIndex + 1));
                return ReadPngDimensionsFromBytes(bytes);
            }
            catch (const std::exception&)
            {
                return Point::Zero;
            }
        }

        Point ReadImageDimensions(
            const std::string& imagePath, const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver)
        {
            // Case-insensitive prefix check, matching upstream's StringComparison.OrdinalIgnoreCase.
            const std::string prefix = "data:image";
            if (imagePath.size() >= prefix.size() &&
                std::equal(prefix.begin(), prefix.end(), imagePath.begin(),
                    [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
            {
                return ReadDimensionsFromDataUri(imagePath);
            }

            try
            {
                const std::string fullPath =
                    System::IO::Path::IsPathRooted(imagePath) ? imagePath : System::IO::Path::Combine(baseDirectory, imagePath);

                const std::unique_ptr<System::IO::Stream> stream = resourceResolver(fullPath);
                std::vector<SharpRuntime::bytecs> header(24);
                const SharpRuntime::intcs bytesRead = stream->Read(header.data(), 0, 24);

                if (bytesRead < 24)
                {
                    return Point::Zero;
                }

                return ReadPngDimensionsFromBytes(header);
            }
            catch (const std::exception&)
            {
                return Point::Zero;
            }
        }

        std::map<std::string, TilesetGidInfo> ConvertTilesets(
            const OgmoProject& project, TilemapData& data, const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver)
        {
            std::map<std::string, TilesetGidInfo> tilesetInfo;

            int currentFirstGid = 1;

            for (const Document::OgmoTilesetTemplate& tilesetTemplate : project.Tilesets)
            {
                if (tilesetTemplate.TileWidth <= 0 || tilesetTemplate.TileHeight <= 0)
                {
                    continue;
                }

                const std::string imagePath = !tilesetTemplate.Image.empty() ? tilesetTemplate.Image : tilesetTemplate.Path;
                if (imagePath.empty())
                {
                    continue;
                }

                const Point imgSize = ReadImageDimensions(imagePath, baseDirectory, resourceResolver);
                const int columns = CalculateCount(imgSize.X, tilesetTemplate.TileWidth, tilesetTemplate.TileSeparationX);
                const int rows = CalculateCount(imgSize.Y, tilesetTemplate.TileHeight, tilesetTemplate.TileSeparationY);
                const int tileCount = columns * rows;

                if (tileCount <= 0)
                {
                    continue;
                }

                TilemapTilesetData tilesetData;
                tilesetData.Name = tilesetTemplate.Label;
                tilesetData.TexturePath = imagePath;
                tilesetData.TileWidth = tilesetTemplate.TileWidth;
                tilesetData.TileHeight = tilesetTemplate.TileHeight;
                tilesetData.TileCount = tileCount;
                tilesetData.Columns = columns;
                tilesetData.Spacing = tilesetTemplate.TileSeparationX;
                tilesetData.Margin = 0;

                TilemapTilesetEntry entry;
                entry.FirstGlobalId = currentFirstGid;
                entry.IsExternal = false;
                entry.InlineData = std::move(tilesetData);

                data.Tilesets.push_back(std::move(entry));
                tilesetInfo[tilesetTemplate.Label] = TilesetGidInfo{currentFirstGid, columns};
                currentFirstGid += tileCount;
            }

            return tilesetInfo;
        }

        void DecodeTileIds1D(TilemapTileLayerData& data, const std::vector<int>& tileIds, int firstGid)
        {
            for (std::size_t i = 0; i < tileIds.size(); ++i)
            {
                const int localId = tileIds[i];
                if (localId < 0)
                {
                    continue;
                }

                const auto x = static_cast<std::uint16_t>(data.Width == 0 ? 0 : i % static_cast<std::size_t>(data.Width));
                const auto y = static_cast<std::uint16_t>(data.Width == 0 ? 0 : i / static_cast<std::size_t>(data.Width));
                data.Tiles.emplace_back(x, y, firstGid + localId, TilemapTileFlipFlags::None);
            }
        }

        void DecodeTileIds2D(TilemapTileLayerData& data, const std::vector<std::vector<int>>& tileIds2D, int firstGid)
        {
            for (std::size_t y = 0; y < tileIds2D.size() && static_cast<int>(y) < data.Height; ++y)
            {
                const std::vector<int>& row = tileIds2D[y];
                for (std::size_t x = 0; x < row.size() && static_cast<int>(x) < data.Width; ++x)
                {
                    const int localId = row[x];
                    if (localId < 0)
                    {
                        continue;
                    }

                    data.Tiles.emplace_back(static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y), firstGid + localId,
                        TilemapTileFlipFlags::None);
                }
            }
        }

        void DecodeTileCoords1D(TilemapTileLayerData& data, const std::vector<std::optional<Point>>& coords, int firstGid, int columns)
        {
            if (columns <= 0)
            {
                return;
            }

            for (std::size_t i = 0; i < coords.size(); ++i)
            {
                if (!coords[i].has_value())
                {
                    continue;
                }

                const int localId = coords[i]->Y * columns + coords[i]->X;
                const auto x = static_cast<std::uint16_t>(data.Width == 0 ? 0 : i % static_cast<std::size_t>(data.Width));
                const auto y = static_cast<std::uint16_t>(data.Width == 0 ? 0 : i / static_cast<std::size_t>(data.Width));
                data.Tiles.emplace_back(x, y, firstGid + localId, TilemapTileFlipFlags::None);
            }
        }

        void DecodeTileCoords2D(
            TilemapTileLayerData& data, const std::vector<std::vector<std::optional<Point>>>& coords2D, int firstGid, int columns)
        {
            if (columns <= 0)
            {
                return;
            }

            for (std::size_t y = 0; y < coords2D.size() && static_cast<int>(y) < data.Height; ++y)
            {
                const std::vector<std::optional<Point>>& row = coords2D[y];
                for (std::size_t x = 0; x < row.size() && static_cast<int>(x) < data.Width; ++x)
                {
                    if (!row[x].has_value())
                    {
                        continue;
                    }

                    const int localId = row[x]->Y * columns + row[x]->X;
                    data.Tiles.emplace_back(static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y), firstGid + localId,
                        TilemapTileFlipFlags::None);
                }
            }
        }

        std::unique_ptr<TilemapTileLayerData> ConvertTileLayer(
            const OgmoTileLayerData& layerData, const OgmoLayerTemplate* layerTemplate, const std::map<std::string, TilesetGidInfo>& tilesetInfo)
        {
            auto result = std::make_unique<TilemapTileLayerData>();
            result->Name = layerData.Name;
            result->Width = layerData.GridCellsX;
            result->Height = layerData.GridCellsY;
            result->IsVisible = true;
            result->Opacity = 1.0f;
            result->OffsetX = layerData.OffsetX;
            result->OffsetY = layerData.OffsetY;
            // Ogmo layers don't have parallax.
            result->ParallaxX = 1.0f;
            result->ParallaxY = 1.0f;

            const std::string tilesetName = !layerData.Tileset.empty() ? layerData.Tileset
                : (layerTemplate != nullptr ? layerTemplate->DefaultTileset : std::string());

            const auto it = tilesetInfo.find(tilesetName);
            if (it == tilesetInfo.end())
            {
                return result;
            }

            const int firstGid = it->second.FirstGid;
            const int columns = it->second.Columns;

            if (layerData.ExportMode == 0)
            {
                if (layerData.ArrayMode == 0)
                {
                    DecodeTileIds1D(*result, layerData.Data, firstGid);
                }
                else
                {
                    DecodeTileIds2D(*result, layerData.Data2D, firstGid);
                }
            }
            else
            {
                if (layerData.ArrayMode == 0)
                {
                    DecodeTileCoords1D(*result, layerData.DataCoords, firstGid, columns);
                }
                else
                {
                    DecodeTileCoords2D(*result, layerData.DataCoords2D, firstGid, columns);
                }
            }

            return result;
        }

        std::unique_ptr<TilemapObjectLayerData> ConvertEntityLayer(const OgmoEntityLayerData& layerData)
        {
            auto result = std::make_unique<TilemapObjectLayerData>();
            result->Name = layerData.Name;
            result->IsVisible = true;
            result->Opacity = 1.0f;
            result->OffsetX = layerData.OffsetX;
            result->OffsetY = layerData.OffsetY;
            result->DrawOrder = TilemapObjectDrawOrder::TopDown;

            for (const OgmoEntity& entity : layerData.Entities)
            {
                auto obj = std::make_unique<TilemapRectangleObjectData>();
                obj->Id = entity.Id;
                obj->Name = entity.Name;
                obj->X = entity.X;
                obj->Y = entity.Y;
                obj->Rotation = entity.Rotation;
                obj->IsVisible = true;
                obj->Width = entity.Width;
                obj->Height = entity.Height;

                if (entity.Values.is_object())
                {
                    for (const auto& [key, jsonValue] : entity.Values.items())
                    {
                        ConvertCustomValue(obj->Properties, key, jsonValue);
                    }
                }

                result->Objects.push_back(std::move(obj));
            }

            return result;
        }

        std::unique_ptr<TilemapObjectLayerData> ConvertDecalLayer(const OgmoDecalLayerData& layerData)
        {
            auto result = std::make_unique<TilemapObjectLayerData>();
            result->Name = layerData.Name;
            result->IsVisible = true;
            result->Opacity = 1.0f;
            result->OffsetX = layerData.OffsetX;
            result->OffsetY = layerData.OffsetY;
            result->ParallaxX = 1.0f;
            result->ParallaxY = 1.0f;
            result->DrawOrder = TilemapObjectDrawOrder::TopDown;

            for (const OgmoDecal& decal : layerData.Decals)
            {
                auto obj = std::make_unique<TilemapRectangleObjectData>();
                obj->Name = decal.Texture;
                obj->X = decal.X;
                obj->Y = decal.Y;
                obj->Rotation = decal.Rotation;
                obj->IsVisible = true;

                // Decals have no fixed size; use a placeholder of 16x16.
                obj->Width = 16;
                obj->Height = 16;

                AddStringProperty(obj->Properties, "Texture", decal.Texture);
                AddStringProperty(obj->Properties, "Folder", layerData.Folder);
                AddFloatProperty(obj->Properties, "ScaleX", decal.ScaleX);
                AddFloatProperty(obj->Properties, "ScaleY", decal.ScaleY);

                if (decal.Values.is_object())
                {
                    for (const auto& [key, jsonValue] : decal.Values.items())
                    {
                        ConvertCustomValue(obj->Properties, key, jsonValue);
                    }
                }

                result->Objects.push_back(std::move(obj));
            }

            return result;
        }

        std::unique_ptr<TilemapTileLayerData> ConvertGridLayer(const OgmoGridLayerData& layerData, const OgmoLayerTemplate* layerTemplate)
        {
            auto result = std::make_unique<TilemapTileLayerData>();
            result->Name = layerData.Name;
            result->Width = layerData.GridCellsX;
            result->Height = layerData.GridCellsY;
            result->IsVisible = true;
            result->Opacity = 1.0f;
            result->OffsetX = layerData.OffsetX;
            result->OffsetY = layerData.OffsetY;
            // Ogmo layers don't have parallax.
            result->ParallaxX = 1.0f;
            result->ParallaxY = 1.0f;

            if (layerData.ArrayMode == 0 && !layerData.Grid.empty())
            {
                const nlohmann::ordered_json gridJson = layerData.Grid;
                AddStringProperty(result->Properties, "Ogmo_GridValues", gridJson.dump());
            }
            else if (layerData.ArrayMode == 1 && !layerData.Grid2D.empty())
            {
                const nlohmann::ordered_json gridJson = layerData.Grid2D;
                AddStringProperty(result->Properties, "Ogmo_GridValues", gridJson.dump());
            }

            AddIntProperty(result->Properties, "Ogmo_GridArrayMode", layerData.ArrayMode);

            if (layerTemplate != nullptr && layerTemplate->Legend.is_object() && !layerTemplate->Legend.empty())
            {
                AddStringProperty(result->Properties, "Ogmo_GridLegend", layerTemplate->Legend.dump());
            }

            return result;
        }

        void ConvertLayers(
            const OgmoLevel& level, const OgmoProject& project, TilemapData& data, const std::map<std::string, TilesetGidInfo>& tilesetInfo)
        {
            for (const std::unique_ptr<OgmoLayerData>& layerData : level.Layers)
            {
                const OgmoLayerTemplate* layerTemplate = FindTemplate(project, layerData->Name);

                std::unique_ptr<TilemapLayerData> converted;

                if (const auto* tileLayerData = dynamic_cast<const OgmoTileLayerData*>(layerData.get()))
                {
                    converted = ConvertTileLayer(*tileLayerData, layerTemplate, tilesetInfo);
                }
                else if (const auto* entityLayerData = dynamic_cast<const OgmoEntityLayerData*>(layerData.get()))
                {
                    converted = ConvertEntityLayer(*entityLayerData);
                }
                else if (const auto* gridLayerData = dynamic_cast<const OgmoGridLayerData*>(layerData.get()))
                {
                    converted = ConvertGridLayer(*gridLayerData, layerTemplate);
                }
                else if (const auto* decalLayerData = dynamic_cast<const OgmoDecalLayerData*>(layerData.get()))
                {
                    converted = ConvertDecalLayer(*decalLayerData);
                }

                if (converted)
                {
                    data.Layers.push_back(std::move(converted));
                }
            }
        }

        void ConvertLevelProperties(const OgmoLevel& level, TilemapData& data)
        {
            AddStringProperty(data.Properties, "Ogmo_Version", level.OgmoVersion);
            AddIntProperty(data.Properties, "Ogmo_PixelWidth", static_cast<int>(level.Width));
            AddIntProperty(data.Properties, "Ogmo_PixelHeight", static_cast<int>(level.Height));
            AddIntProperty(data.Properties, "Ogmo_OffsetX", static_cast<int>(level.OffsetX));
            AddIntProperty(data.Properties, "Ogmo_OffsetY", static_cast<int>(level.OffsetY));

            if (level.Values.is_object())
            {
                for (const auto& [key, jsonValue] : level.Values.items())
                {
                    ConvertCustomValue(data.Properties, "Level_" + key, jsonValue);
                }
            }
        }
    }

    TilemapData Convert(const OgmoLevel& level, const OgmoProject& project)
    {
        return Convert(level, project, std::filesystem::current_path().string(), Content::ExternalResourceResolver(&Content::OpenFile));
    }

    TilemapData Convert(
        const OgmoLevel& level, const OgmoProject& project, const std::string& baseDirectory, const Content::ExternalResourceResolver& resourceResolver)
    {
        int gridSize = static_cast<int>(project.LayerGridDefaultSize.has_value() ? project.LayerGridDefaultSize->X : 16.0f);
        if (gridSize <= 0)
        {
            gridSize = 16;
        }

        // Upstream distinguishes a missing "ogmoVersion" JSON field (deserializes to a null C#
        // string, so Path.GetFileNameWithoutExtension(null) ?? "Ogmo Level" falls back) from a
        // present-but-empty one (Path.GetFileNameWithoutExtension("") == "", no fallback) --
        // std::string has no null state, so this port collapses both cases to the same fallback.
        // A minor, deliberate simplification of an already-obscure quirk (see this file's top
        // comment), not expected to matter for any real Ogmo project file.
        TilemapData data;
        data.Name = !level.OgmoVersion.empty() ? System::IO::Path::GetFileNameWithoutExtension(level.OgmoVersion) : "Ogmo Level";
        data.Width = static_cast<int>(level.Width / static_cast<float>(gridSize));
        data.Height = static_cast<int>(level.Height / static_cast<float>(gridSize));
        data.TileWidth = gridSize;
        data.TileHeight = gridSize;
        // Ogmo only supports orthogonal maps.
        data.Orientation = TilemapOrientation::Orthogonal;

        if (!project.BackgroundColor.empty())
        {
            data.BackgroundColor = ParseColor(project.BackgroundColor);
        }

        const std::map<std::string, TilesetGidInfo> tilesetInfo = ConvertTilesets(project, data, baseDirectory, resourceResolver);
        ConvertLayers(level, project, data, tilesetInfo);
        ConvertLevelProperties(level, data);

        return data;
    }
}
