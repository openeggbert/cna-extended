// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Ogmo/Document/OgmoDocument.hpp"

namespace CNA::Extended::Tilemaps::Ogmo::Document
{
    namespace
    {
        template <typename T>
        void ReadOpt(const nlohmann::ordered_json& j, const char* key, std::optional<T>& target)
        {
            if (j.contains(key) && !j.at(key).is_null())
            {
                target = j.at(key).get<T>();
            }
        }

        template <typename T>
        void ReadOr(const nlohmann::ordered_json& j, const char* key, T& target, const T& fallback)
        {
            if (j.contains(key) && !j.at(key).is_null())
            {
                target = j.at(key).get<T>();
            }
            else
            {
                target = fallback;
            }
        }

        template <typename T>
        void ReadVec(const nlohmann::ordered_json& j, const char* key, std::vector<T>& target)
        {
            target.clear();
            if (j.contains(key) && j.at(key).is_array())
            {
                for (const auto& element : j.at(key))
                {
                    target.push_back(element.get<T>());
                }
            }
        }

        void ReadJson(const nlohmann::ordered_json& j, const char* key, nlohmann::ordered_json& target)
        {
            if (j.contains(key) && !j.at(key).is_null())
            {
                target = j.at(key);
            }
            else
            {
                target = nlohmann::ordered_json::object();
            }
        }
    }

    void from_json(const nlohmann::ordered_json& j, OgmoVector2& value)
    {
        ReadOr(j, "x", value.X, 0.0f);
        ReadOr(j, "y", value.Y, 0.0f);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoShape& value)
    {
        ReadOr(j, "label", value.Label, std::string());
        ReadVec(j, "points", value.Points);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoValueTemplate& value)
    {
        ReadOr(j, "name", value.Name, std::string());
        ReadOr(j, "definition", value.Definition, std::string());
        ReadJson(j, "defaults", value.Defaults);
        ReadOr(j, "bounded", value.Bounded, false);
        ReadOpt(j, "min", value.Min);
        ReadOpt(j, "max", value.Max);
        ReadOpt(j, "maxLength", value.MaxLength);
        ReadOr(j, "trimWhitespace", value.TrimWhitespace, false);
        ReadVec(j, "choices", value.Choices);
        ReadOr(j, "includeAlpha", value.IncludeAlpha, false);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoTilesetTemplate& value)
    {
        ReadOr(j, "label", value.Label, std::string());
        ReadOr(j, "path", value.Path, std::string());
        ReadOr(j, "image", value.Image, std::string());
        ReadOr(j, "tileWidth", value.TileWidth, 0);
        ReadOr(j, "tileHeight", value.TileHeight, 0);
        ReadOr(j, "tileSeparationX", value.TileSeparationX, 0);
        ReadOr(j, "tileSeparationY", value.TileSeparationY, 0);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoEntityTemplate& value)
    {
        ReadOr(j, "name", value.Name, std::string());
        ReadOr(j, "exportID", value.ExportID, std::string());
        ReadOr(j, "limit", value.Limit, 0);
        ReadOr(j, "size", value.Size, OgmoVector2());
        ReadOr(j, "origin", value.Origin, OgmoVector2());
        ReadOr(j, "originAnchored", value.OriginAnchored, false);
        ReadOr(j, "shape", value.Shape, OgmoShape());
        ReadOr(j, "color", value.Color, std::string());
        ReadOr(j, "tileX", value.TileX, false);
        ReadOr(j, "tileY", value.TileY, false);
        ReadOr(j, "tileSize", value.TileSize, OgmoVector2());
        ReadOr(j, "resizeableX", value.ResizeableX, false);
        ReadOr(j, "resizeableY", value.ResizeableY, false);
        ReadOr(j, "rotatable", value.Rotatable, false);
        ReadOr(j, "rotationDegrees", value.RotationDegrees, 0.0f);
        ReadOr(j, "canFlipX", value.CanFlipX, false);
        ReadOr(j, "canFlipY", value.CanFlipY, false);
        ReadOr(j, "canSetColor", value.CanSetColor, false);
        ReadOr(j, "hasNodes", value.HasNodes, false);
        ReadOr(j, "nodeLimit", value.NodeLimit, 0);
        ReadOr(j, "nodeDisplay", value.NodeDisplay, 0);
        ReadOr(j, "nodeGhost", value.NodeGhost, false);
        ReadVec(j, "tags", value.Tags);
        ReadVec(j, "values", value.Values);
        ReadOr(j, "texture", value.Texture, std::string());
        ReadOr(j, "textureImage", value.TextureImage, std::string());
    }

    void from_json(const nlohmann::ordered_json& j, OgmoLayerTemplate& value)
    {
        ReadOr(j, "name", value.Name, std::string());
        ReadOr(j, "definition", value.Definition, std::string());
        ReadOr(j, "gridSize", value.GridSize, OgmoVector2());
        ReadOr(j, "exportID", value.ExportID, std::string());
        ReadOr(j, "exportMode", value.ExportMode, 0);
        ReadOr(j, "arrayMode", value.ArrayMode, 0);
        ReadOr(j, "defaultTileset", value.DefaultTileset, std::string());
        ReadJson(j, "legend", value.Legend);
        ReadVec(j, "requiredTags", value.RequiredTags);
        ReadVec(j, "excludedTags", value.ExcludedTags);
        ReadOr(j, "folder", value.Folder, std::string());
        ReadOr(j, "includeImageSequence", value.IncludeImageSequence, false);
        ReadOr(j, "scaleable", value.Scaleable, false);
        ReadOr(j, "rotatable", value.Rotatable, false);
        ReadVec(j, "values", value.Values);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoProject& value)
    {
        ReadOr(j, "name", value.Name, std::string());
        ReadOr(j, "ogmoVersion", value.OgmoVersion, std::string());
        ReadVec(j, "levelPaths", value.LevelPaths);
        ReadOr(j, "backgroundColor", value.BackgroundColor, std::string());
        ReadOr(j, "gridColor", value.GridColor, std::string());
        ReadOr(j, "anglesRadians", value.AnglesRadians, false);
        ReadOr(j, "directoryDepth", value.DirectoryDepth, 0);
        ReadOpt(j, "layerGridDefaultSize", value.LayerGridDefaultSize);
        ReadOpt(j, "levelDefaultSize", value.LevelDefaultSize);
        ReadOpt(j, "levelMinSize", value.LevelMinSize);
        ReadOpt(j, "levelMaxSize", value.LevelMaxSize);
        ReadVec(j, "levelValues", value.LevelValues);
        ReadOr(j, "defaultExportMode", value.DefaultExportMode, std::string());
        ReadOr(j, "compactExport", value.CompactExport, false);
        ReadVec(j, "entityTags", value.EntityTags);
        ReadVec(j, "layers", value.Layers);
        ReadVec(j, "entities", value.Entities);
        ReadVec(j, "tilesets", value.Tilesets);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoEntity& value)
    {
        ReadOr(j, "name", value.Name, std::string());
        ReadOr(j, "id", value.Id, 0);
        ReadOr(j, "_eid", value.ExportID, std::string());
        ReadOr(j, "x", value.X, 0.0f);
        ReadOr(j, "y", value.Y, 0.0f);
        ReadOr(j, "width", value.Width, 0.0f);
        ReadOr(j, "height", value.Height, 0.0f);
        ReadOr(j, "originX", value.OriginX, 0.0f);
        ReadOr(j, "originY", value.OriginY, 0.0f);
        ReadOr(j, "rotation", value.Rotation, 0.0f);
        ReadOr(j, "flippedX", value.FlippedX, false);
        ReadOr(j, "flippedY", value.FlippedY, false);
        ReadVec(j, "nodes", value.Nodes);
        ReadJson(j, "values", value.Values);
    }

    void from_json(const nlohmann::ordered_json& j, OgmoDecal& value)
    {
        ReadOr(j, "x", value.X, 0.0f);
        ReadOr(j, "y", value.Y, 0.0f);
        ReadOr(j, "texture", value.Texture, std::string());
        ReadOr(j, "rotation", value.Rotation, 0.0f);
        ReadOr(j, "scaleX", value.ScaleX, 0.0f);
        ReadOr(j, "scaleY", value.ScaleY, 0.0f);
        ReadJson(j, "values", value.Values);
    }

    namespace
    {
        void ReadLayerDataBase(const nlohmann::ordered_json& j, OgmoLayerData& value)
        {
            ReadOr(j, "name", value.Name, std::string());
            ReadOr(j, "_eid", value.ExportID, std::string());
            ReadOr(j, "offsetX", value.OffsetX, 0.0f);
            ReadOr(j, "offsetY", value.OffsetY, 0.0f);
            ReadOr(j, "gridCellWidth", value.GridCellWidth, 0);
            ReadOr(j, "gridCellHeight", value.GridCellHeight, 0);
            ReadOr(j, "gridCellsX", value.GridCellsX, 0);
            ReadOr(j, "gridCellsY", value.GridCellsY, 0);
        }
    }

    std::optional<Point> ParseCoordPair(const nlohmann::ordered_json& pairArray)
    {
        if (!pairArray.is_array())
        {
            return std::nullopt;
        }

        int x = -1;
        int y = -1;
        std::size_t count = 0;

        for (const auto& element : pairArray)
        {
            if (count == 0)
            {
                x = element.get<int>();
            }
            else if (count == 1)
            {
                y = element.get<int>();
            }
            ++count;
        }

        // A pair with two elements is a valid [tileX, tileY] coordinate. A single-element array
        // (count == 1, the "[-1]" sentinel) means empty regardless of the value.
        if (count >= 2)
        {
            return Point(x, y);
        }

        return std::nullopt;
    }

    std::unique_ptr<OgmoLayerData> ParseOgmoLayerData(const nlohmann::ordered_json& j)
    {
        if (j.contains("tileset") || j.contains("data") || j.contains("data2D"))
        {
            auto layer = std::make_unique<OgmoTileLayerData>();
            ReadLayerDataBase(j, *layer);
            ReadOr(j, "tileset", layer->Tileset, std::string());
            ReadVec(j, "data", layer->Data);

            layer->Data2D.clear();
            if (j.contains("data2D") && j.at("data2D").is_array())
            {
                for (const auto& row : j.at("data2D"))
                {
                    layer->Data2D.push_back(row.get<std::vector<int>>());
                }
            }

            layer->DataCoords.clear();
            if (j.contains("dataCoords") && j.at("dataCoords").is_array())
            {
                for (const auto& pairArray : j.at("dataCoords"))
                {
                    layer->DataCoords.push_back(ParseCoordPair(pairArray));
                }
            }

            layer->DataCoords2D.clear();
            if (j.contains("dataCoords2D") && j.at("dataCoords2D").is_array())
            {
                for (const auto& row : j.at("dataCoords2D"))
                {
                    std::vector<std::optional<Point>> parsedRow;
                    if (row.is_array())
                    {
                        for (const auto& pairArray : row)
                        {
                            parsedRow.push_back(ParseCoordPair(pairArray));
                        }
                    }
                    layer->DataCoords2D.push_back(std::move(parsedRow));
                }
            }

            ReadOr(j, "exportMode", layer->ExportMode, 0);
            ReadOr(j, "arrayMode", layer->ArrayMode, 0);
            return layer;
        }

        if (j.contains("grid") || j.contains("grid2D"))
        {
            auto layer = std::make_unique<OgmoGridLayerData>();
            ReadLayerDataBase(j, *layer);
            ReadVec(j, "grid", layer->Grid);

            layer->Grid2D.clear();
            if (j.contains("grid2D") && j.at("grid2D").is_array())
            {
                for (const auto& row : j.at("grid2D"))
                {
                    layer->Grid2D.push_back(row.get<std::vector<std::string>>());
                }
            }

            ReadOr(j, "arrayMode", layer->ArrayMode, 0);
            return layer;
        }

        if (j.contains("entities"))
        {
            auto layer = std::make_unique<OgmoEntityLayerData>();
            ReadLayerDataBase(j, *layer);
            ReadVec(j, "entities", layer->Entities);
            return layer;
        }

        if (j.contains("decals"))
        {
            auto layer = std::make_unique<OgmoDecalLayerData>();
            ReadLayerDataBase(j, *layer);
            ReadVec(j, "decals", layer->Decals);
            ReadOr(j, "folder", layer->Folder, std::string());
            return layer;
        }

        // Could not determine the concrete type: return base layer data, matching upstream
        // OgmoLayerDataConverter.Read's own fallback.
        auto layer = std::make_unique<OgmoLayerData>();
        ReadLayerDataBase(j, *layer);
        return layer;
    }

    void from_json(const nlohmann::ordered_json& j, OgmoLevel& value)
    {
        ReadOr(j, "ogmoVersion", value.OgmoVersion, std::string());
        ReadOr(j, "width", value.Width, 0.0f);
        ReadOr(j, "height", value.Height, 0.0f);
        ReadOr(j, "offsetX", value.OffsetX, 0.0f);
        ReadOr(j, "offsetY", value.OffsetY, 0.0f);
        ReadJson(j, "values", value.Values);

        value.Layers.clear();
        if (j.contains("layers") && j.at("layers").is_array())
        {
            for (const auto& layerJson : j.at("layers"))
            {
                value.Layers.push_back(ParseOgmoLayerData(layerJson));
            }
        }
    }
}
