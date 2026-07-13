// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/LDtk/Converters/LDtkTilemapDataConverter.cs.
//
// `entity.Iid.GetHashCode()` (used to derive an int TilemapObjectData::Id from an LDtk entity's
// GUID-like `iid` string) has no faithfully-portable C++ equivalent: .NET's `string.GetHashCode()`
// is not a fixed algorithm across .NET versions and is randomized per-process by default (it was
// never meant to be a stable, reproducible value even within upstream itself). `std::hash<
// std::string>` is used here as the direct structural analogue -- "derive an int from the string
// for local uniqueness," not "reproduce .NET's specific hash output" -- matching the same spirit
// as upstream's own code, which never relied on hash *stability* either.
#include "CNA/Extended/Tilemaps/LDtk/Converters/LDtkTilemapDataConverter.hpp"

#include "CNA/Extended/Tilemaps/LDtk/LDtkColorParser.hpp"
#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"

#include <cstdint>
#include <sstream>
#include <unordered_map>

namespace CNA::Extended::Tilemaps::LDtk::Converters
{
    using Parsers::TilemapParseException;

    namespace
    {
        void AddStringProperty(std::vector<TilemapPropertyData>& target, const std::string& key, const std::string& value)
        {
            TilemapPropertyData prop;
            prop.Key = key;
            prop.Type = TilemapPropertyType::String;
            prop.StringValue = value;
            target.push_back(std::move(prop));
        }

        void AddIntProperty(std::vector<TilemapPropertyData>& target, const std::string& key, int value)
        {
            TilemapPropertyData prop;
            prop.Key = key;
            prop.Type = TilemapPropertyType::Int;
            prop.IntValue = value;
            target.push_back(std::move(prop));
        }

        void AddFloatProperty(std::vector<TilemapPropertyData>& target, const std::string& key, float value)
        {
            TilemapPropertyData prop;
            prop.Key = key;
            prop.Type = TilemapPropertyType::Float;
            prop.FloatValue = value;
            target.push_back(std::move(prop));
        }

        void AddBoolProperty(std::vector<TilemapPropertyData>& target, const std::string& key, bool value)
        {
            TilemapPropertyData prop;
            prop.Key = key;
            prop.Type = TilemapPropertyType::Bool;
            prop.BoolValue = value;
            target.push_back(std::move(prop));
        }

        void AddColorProperty(std::vector<TilemapPropertyData>& target, const std::string& key, const Color& value)
        {
            TilemapPropertyData prop;
            prop.Key = key;
            prop.Type = TilemapPropertyType::Color;
            prop.ColorValue = value;
            target.push_back(std::move(prop));
        }

        // --- GID and Flip Conversion --------------------------------------------------------

        int ConvertToGlobalId(int localId, const std::unordered_map<int, int>& tilesetFirstGids, const std::optional<int>& tilesetDefUid)
        {
            if (tilesetDefUid.has_value())
            {
                const auto it = tilesetFirstGids.find(*tilesetDefUid);
                if (it != tilesetFirstGids.end())
                {
                    return it->second + localId;
                }
            }
            return localId;
        }

        TilemapTileFlipFlags ConvertFlipFlags(int ldtkFlags)
        {
            TilemapTileFlipFlags flags = TilemapTileFlipFlags::None;

            if ((ldtkFlags & 1) != 0)
            {
                flags |= TilemapTileFlipFlags::FlipHorizontally;
            }

            if ((ldtkFlags & 2) != 0)
            {
                flags |= TilemapTileFlipFlags::FlipVertically;
            }

            // LDtk does not support diagonal flip.

            return flags;
        }

        // --- Field Instance Conversion -------------------------------------------------------

        void ConvertFieldInstance(const Document::LDtkFieldInstance& field, std::vector<TilemapPropertyData>& target)
        {
            if (!field.Value.has_value())
            {
                AddStringProperty(target, field.Identifier, "");
                return;
            }

            const nlohmann::ordered_json& jsonValue = *field.Value;

            if (field.Type == "Int")
            {
                if (jsonValue.is_number())
                {
                    AddIntProperty(target, field.Identifier, jsonValue.get<int>());
                }
                else
                {
                    AddStringProperty(target, field.Identifier, jsonValue.dump());
                }
            }
            else if (field.Type == "Float")
            {
                if (jsonValue.is_number())
                {
                    AddFloatProperty(target, field.Identifier, jsonValue.get<float>());
                }
                else
                {
                    AddStringProperty(target, field.Identifier, jsonValue.dump());
                }
            }
            else if (field.Type == "Bool")
            {
                if (jsonValue.is_boolean())
                {
                    AddBoolProperty(target, field.Identifier, jsonValue.get<bool>());
                }
                else
                {
                    AddStringProperty(target, field.Identifier, jsonValue.dump());
                }
            }
            else if (field.Type == "Color")
            {
                if (jsonValue.is_string())
                {
                    const Color color = ParseColor(jsonValue.get<std::string>());
                    AddColorProperty(target, field.Identifier, color);
                }
                else
                {
                    AddStringProperty(target, field.Identifier, jsonValue.dump());
                }
            }
            else if (field.Type == "EntityRef")
            {
                if (jsonValue.is_object() && jsonValue.contains("entityIid"))
                {
                    AddStringProperty(target, field.Identifier, jsonValue.at("entityIid").get<std::string>());
                }
                else
                {
                    AddStringProperty(target, field.Identifier, "");
                }
            }
            else
            {
                // String, Text, Multilines, FilePath, Point, and any unknown types are serialized as string.
                if (jsonValue.is_string())
                {
                    AddStringProperty(target, field.Identifier, jsonValue.get<std::string>());
                }
                else
                {
                    AddStringProperty(target, field.Identifier, jsonValue.dump());
                }
            }
        }

        void ConvertFieldInstances(const std::vector<Document::LDtkFieldInstance>& fieldInstances, std::vector<TilemapPropertyData>& target)
        {
            for (const Document::LDtkFieldInstance& field : fieldInstances)
            {
                if (field.Identifier.empty())
                {
                    continue;
                }

                try
                {
                    ConvertFieldInstance(field, target);
                }
                catch (const std::exception& ex)
                {
                    throw TilemapParseException(
                        "Failed to convert LDtk field '" + field.Identifier + "' of type '" + field.Type + "': " + ex.what());
                }
            }
        }

        // --- Layer Helpers -------------------------------------------------------------------

        void ApplyLayerBase(TilemapLayerData& target, const Document::LDtkLayerInstance& source)
        {
            target.Name = source.Identifier;
            target.Class.clear();
            target.IsVisible = source.Visible;
            target.Opacity = source.Opacity;
            target.OffsetX = static_cast<float>(source.PxOffsetX);
            target.OffsetY = static_cast<float>(source.PxOffsetY);

            // LDtk parallax: 1 + LDtk factor = Tiled equivalent (0 = stationary, 1 = normal, 2 = 2x speed).
            target.ParallaxX = 1.0f + source.ParallaxFactorX;
            target.ParallaxY = 1.0f + source.ParallaxFactorY;

            // LDtk layers don't have tint color.
            target.TintColor.reset();

            AddIntProperty(target.Properties, "LDtk_Uid", source.LayerDefUid);
            AddStringProperty(target.Properties, "LDtk_Type", source.Type);
            AddIntProperty(target.Properties, "LDtk_GridSize", source.GridSize);
        }

        std::unordered_map<std::int64_t, std::vector<const Document::LDtkTileInstance*>> GroupTilesByCell(
            const std::vector<Document::LDtkTileInstance>& tiles, int gridSize)
        {
            std::unordered_map<std::int64_t, std::vector<const Document::LDtkTileInstance*>> tilesByCell;

            if (gridSize <= 0)
            {
                return tilesByCell;
            }

            for (const Document::LDtkTileInstance& tile : tiles)
            {
                if (tile.Px.size() < 2)
                {
                    continue;
                }

                const int cellX = tile.Px[0] / gridSize;
                const int cellY = tile.Px[1] / gridSize;
                const std::int64_t key = (static_cast<std::int64_t>(cellY) << 32) | static_cast<std::uint32_t>(cellX);

                tilesByCell[key].push_back(&tile);
            }

            return tilesByCell;
        }

        std::vector<std::unique_ptr<TilemapLayerData>> CreateSubLayers(
            const std::unordered_map<std::int64_t, std::vector<const Document::LDtkTileInstance*>>& tilesByCell, const std::string& baseName,
            int gridWidth, int gridHeight, const std::unordered_map<int, int>& tilesetFirstGids, const std::optional<int>& tilesetDefUid,
            const Document::LDtkLayerInstance& layerInstance)
        {
            std::size_t maxDepth = 0;
            for (const auto& [key, cell] : tilesByCell)
            {
                maxDepth = std::max(maxDepth, cell.size());
            }

            std::vector<std::unique_ptr<TilemapLayerData>> result;

            for (std::size_t depth = 0; depth < maxDepth; ++depth)
            {
                const std::string subName = maxDepth == 1 ? baseName : baseName + "_Sub" + std::to_string(depth);

                auto layerData = std::make_unique<TilemapTileLayerData>();
                layerData->Width = gridWidth;
                layerData->Height = gridHeight;

                ApplyLayerBase(*layerData, layerInstance);
                layerData->Name = subName;

                for (const auto& [key, cell] : tilesByCell)
                {
                    if (depth >= cell.size())
                    {
                        continue;
                    }

                    const Document::LDtkTileInstance* tile = cell[depth];
                    const auto cellX = static_cast<int>(static_cast<std::uint32_t>(key & 0xFFFFFFFFULL));
                    const auto cellY = static_cast<int>(key >> 32);
                    const int globalId = ConvertToGlobalId(tile->TileId, tilesetFirstGids, tilesetDefUid);

                    const TilemapTileFlipFlags flags = ConvertFlipFlags(tile->FlipFlags);
                    layerData->Tiles.emplace_back(static_cast<std::uint16_t>(cellX), static_cast<std::uint16_t>(cellY), globalId, flags);
                }

                result.push_back(std::move(layerData));
            }

            return result;
        }

        std::unique_ptr<TilemapTileLayerData> CreateEmptyTileLayer(
            const std::string& name, int gridWidth, int gridHeight, const Document::LDtkLayerInstance& layerInstance)
        {
            auto layerData = std::make_unique<TilemapTileLayerData>();
            layerData->Width = gridWidth;
            layerData->Height = gridHeight;
            ApplyLayerBase(*layerData, layerInstance);
            layerData->Name = name;
            return layerData;
        }

        // --- Layers ----------------------------------------------------------------------------

        std::vector<std::unique_ptr<TilemapLayerData>> ConvertTileLayer(
            const Document::LDtkLayerInstance& layerInstance, const std::unordered_map<int, int>& tilesetFirstGids)
        {
            const int gridWidth = layerInstance.CWid;
            const int gridHeight = layerInstance.CHei;
            const std::string name = layerInstance.Identifier.empty() ? "Untitled" : layerInstance.Identifier;

            // LDtk allows multiple manually placed tiles at a single grid cell.
            // Group tiles by cell to determine how many sub-layers are needed.
            const auto tilesByCell = GroupTilesByCell(layerInstance.GridTiles, layerInstance.GridSize);

            std::vector<std::unique_ptr<TilemapLayerData>> result =
                CreateSubLayers(tilesByCell, name, gridWidth, gridHeight, tilesetFirstGids, layerInstance.TilesetDefUid, layerInstance);

            // AutoLayerTiles on a Tiles layer sit on top of the manually placed tiles.
            if (!layerInstance.AutoLayerTiles.empty())
            {
                const auto autoTilesByCell = GroupTilesByCell(layerInstance.AutoLayerTiles, layerInstance.GridSize);

                std::vector<std::unique_ptr<TilemapLayerData>> autoLayers = CreateSubLayers(
                    autoTilesByCell, name + "_Auto", gridWidth, gridHeight, tilesetFirstGids, layerInstance.TilesetDefUid, layerInstance);

                for (std::unique_ptr<TilemapLayerData>& autoLayer : autoLayers)
                {
                    result.push_back(std::move(autoLayer));
                }
            }

            if (result.empty())
            {
                result.push_back(CreateEmptyTileLayer(name, gridWidth, gridHeight, layerInstance));
            }

            return result;
        }

        std::vector<std::unique_ptr<TilemapLayerData>> ConvertAutoLayer(
            const Document::LDtkLayerInstance& layerInstance, const std::unordered_map<int, int>& tilesetFirstGids)
        {
            const int gridWidth = layerInstance.CWid;
            const int gridHeight = layerInstance.CHei;
            const std::string name = layerInstance.Identifier.empty() ? "Untitled" : layerInstance.Identifier;

            const auto tilesByCell = GroupTilesByCell(layerInstance.AutoLayerTiles, layerInstance.GridSize);

            std::vector<std::unique_ptr<TilemapLayerData>> result =
                CreateSubLayers(tilesByCell, name, gridWidth, gridHeight, tilesetFirstGids, layerInstance.TilesetDefUid, layerInstance);

            if (result.empty())
            {
                result.push_back(CreateEmptyTileLayer(name, gridWidth, gridHeight, layerInstance));
            }

            return result;
        }

        std::vector<std::unique_ptr<TilemapLayerData>> ConvertIntGridLayer(
            const Document::LDtkLayerInstance& layerInstance, const std::unordered_map<int, int>& tilesetFirstGids)
        {
            std::vector<std::unique_ptr<TilemapLayerData>> result;

            const int gridWidth = layerInstance.CWid;
            const int gridHeight = layerInstance.CHei;
            const std::string name = layerInstance.Identifier.empty() ? "Untitled" : layerInstance.Identifier;

            auto dataLayer = std::make_unique<TilemapDataLayerData>();
            dataLayer->Width = gridWidth;
            dataLayer->Height = gridHeight;
            ApplyLayerBase(*dataLayer, layerInstance);

            if (!layerInstance.IntGridCsv.empty())
            {
                // IntGrid values are always a data concern, regardless of whether the layer
                // also has a visual auto-tileset. The auto-tiles are pre-baked by LDtk at
                // export time and do not depend on the IntGrid values at parse time.
                std::ostringstream csv;
                for (std::size_t i = 0; i < layerInstance.IntGridCsv.size(); ++i)
                {
                    if (i > 0)
                    {
                        csv << ',';
                    }
                    csv << layerInstance.IntGridCsv[i];
                }
                AddStringProperty(dataLayer->Properties, "LDtk_IntGridCsv", csv.str());
            }

            result.push_back(std::move(dataLayer));

            if (!layerInstance.AutoLayerTiles.empty())
            {
                const auto tilesByCell = GroupTilesByCell(layerInstance.AutoLayerTiles, layerInstance.GridSize);

                std::vector<std::unique_ptr<TilemapLayerData>> autoLayers =
                    CreateSubLayers(tilesByCell, name, gridWidth, gridHeight, tilesetFirstGids, layerInstance.TilesetDefUid, layerInstance);

                for (std::unique_ptr<TilemapLayerData>& autoLayer : autoLayers)
                {
                    result.push_back(std::move(autoLayer));
                }
            }

            return result;
        }

        std::unique_ptr<TilemapObjectLayerData> ConvertEntityLayer(const Document::LDtkLayerInstance& layerInstance)
        {
            auto data = std::make_unique<TilemapObjectLayerData>();
            data->DrawOrder = TilemapObjectDrawOrder::TopDown;
            ApplyLayerBase(*data, layerInstance);

            for (const Document::LDtkEntityInstance& entity : layerInstance.EntityInstances)
            {
                auto obj = std::make_unique<TilemapRectangleObjectData>();
                // LDtk entity IIDs are GUIDs; hash to an int for cross-format compatibility.
                obj->Id = entity.Iid.empty() ? 0 : static_cast<int>(std::hash<std::string>{}(entity.Iid));
                obj->Name = entity.Identifier;
                std::ostringstream tagsJoined;
                for (std::size_t i = 0; i < entity.Tags.size(); ++i)
                {
                    if (i > 0)
                    {
                        tagsJoined << ',';
                    }
                    tagsJoined << entity.Tags[i];
                }
                obj->Class = tagsJoined.str();
                obj->X = entity.Px.size() > 0 ? static_cast<float>(entity.Px[0]) : 0.0f;
                obj->Y = entity.Px.size() > 1 ? static_cast<float>(entity.Px[1]) : 0.0f;
                obj->Rotation = 0.0f;
                obj->IsVisible = true;
                obj->Width = static_cast<float>(entity.Width);
                obj->Height = static_cast<float>(entity.Height);

                AddIntProperty(obj->Properties, "LDtk_DefUid", entity.DefUid);
                AddStringProperty(obj->Properties, "LDtk_Iid", entity.Iid);

                if (!entity.FieldInstances.empty())
                {
                    ConvertFieldInstances(entity.FieldInstances, obj->Properties);
                }

                data->Objects.push_back(std::move(obj));
            }

            return data;
        }

        std::vector<std::unique_ptr<TilemapLayerData>> ConvertLayerInstance(
            const Document::LDtkLayerInstance& layerInstance, const std::unordered_map<int, int>& tilesetFirstGids)
        {
            if (layerInstance.Type == "Tiles")
            {
                return ConvertTileLayer(layerInstance, tilesetFirstGids);
            }
            if (layerInstance.Type == "AutoLayer")
            {
                return ConvertAutoLayer(layerInstance, tilesetFirstGids);
            }
            if (layerInstance.Type == "IntGrid")
            {
                return ConvertIntGridLayer(layerInstance, tilesetFirstGids);
            }
            if (layerInstance.Type == "Entities")
            {
                std::vector<std::unique_ptr<TilemapLayerData>> result;
                result.push_back(ConvertEntityLayer(layerInstance));
                return result;
            }
            return {};
        }

        void ConvertLayers(const Document::LDtkLevel& level, TilemapData& data, const std::unordered_map<int, int>& tilesetFirstGids)
        {
            if (!level.LayerInstances.has_value() || level.LayerInstances->empty())
            {
                return;
            }

            // LDtk layers are ordered top-to-bottom; reverse to bottom-to-top for the runtime.
            const std::vector<Document::LDtkLayerInstance>& layers = *level.LayerInstances;
            for (auto it = layers.rbegin(); it != layers.rend(); ++it)
            {
                std::vector<std::unique_ptr<TilemapLayerData>> converted = ConvertLayerInstance(*it, tilesetFirstGids);

                for (std::unique_ptr<TilemapLayerData>& layer : converted)
                {
                    data.Layers.push_back(std::move(layer));
                }
            }
        }

        // --- Tilesets --------------------------------------------------------------------------

        std::unordered_map<int, int> ConvertTilesets(const Document::LDtkProject& project, TilemapData& data)
        {
            std::unordered_map<int, int> tilesetFirstGids;

            if (!project.Defs.has_value())
            {
                return tilesetFirstGids;
            }

            // GIDs start at 1 because 0 is the universally reserved empty tile sentinel,
            // keeping the global ID space consistent across both Tiled and LDtk formats.
            int currentFirstGid = 1;

            for (const Document::LDtkTilesetDefinition& tilesetDef : project.Defs->Tilesets)
            {
                if (tilesetDef.TileGridSize <= 0)
                {
                    continue;
                }

                // Skip embedded tilesets that have no external image file (e.g., LDtk's Internal_Icons).
                if (tilesetDef.RelPath.empty())
                {
                    continue;
                }

                const int columns = tilesetDef.PxWid / tilesetDef.TileGridSize;
                const int rows = tilesetDef.PxHei / tilesetDef.TileGridSize;
                const int tileCount = columns * rows;

                TilemapTilesetData tilesetData;
                tilesetData.Name = tilesetDef.Identifier;
                tilesetData.TexturePath = tilesetDef.RelPath;
                tilesetData.TileWidth = tilesetDef.TileGridSize;
                tilesetData.TileHeight = tilesetDef.TileGridSize;
                tilesetData.TileCount = tileCount;
                tilesetData.Columns = columns;
                tilesetData.Spacing = tilesetDef.Spacing;
                tilesetData.Margin = tilesetDef.Padding;

                AddIntProperty(tilesetData.Properties, "LDtk_Uid", tilesetDef.Uid);
                AddStringProperty(tilesetData.Properties, "LDtk_RelPath", tilesetDef.RelPath);

                TilemapTilesetEntry entry;
                entry.FirstGlobalId = currentFirstGid;
                entry.IsExternal = false;
                entry.InlineData = std::move(tilesetData);

                data.Tilesets.push_back(std::move(entry));
                tilesetFirstGids[tilesetDef.Uid] = currentFirstGid;
                currentFirstGid += tileCount;
            }

            return tilesetFirstGids;
        }

        // --- Level Properties --------------------------------------------------------------------

        void ConvertLevelProperties(const Document::LDtkLevel& level, TilemapData& data)
        {
            AddStringProperty(data.Properties, "LDtk_Iid", level.Iid);
            AddIntProperty(data.Properties, "LDtk_Uid", level.Uid);
            AddIntProperty(data.Properties, "LDtk_WorldX", level.WorldX);
            AddIntProperty(data.Properties, "LDtk_WorldY", level.WorldY);
            AddIntProperty(data.Properties, "LDtk_WorldDepth", level.WorldDepth);
            data.WorldX = level.WorldX;
            data.WorldY = level.WorldY;
            data.WorldDepth = level.WorldDepth;

            if (!level.Neighbours.empty())
            {
                std::ostringstream sb;
                sb << '[';
                for (std::size_t i = 0; i < level.Neighbours.size(); ++i)
                {
                    const Document::LDtkNeighbourLevel& neighbour = level.Neighbours[i];
                    if (i > 0)
                    {
                        sb << ',';
                    }
                    sb << "{\"dir\":\"" << neighbour.Dir << "\",\"levelIid\":\"" << neighbour.LevelIid << "\"}";
                }
                sb << ']';
                AddStringProperty(data.Properties, "LDtk_Neighbours", sb.str());
            }

            if (!level.FieldInstances.empty())
            {
                ConvertFieldInstances(level.FieldInstances, data.Properties);
            }
        }

        // --- Level Size ------------------------------------------------------------------------

        int GetLevelGridSize(const Document::LDtkLevel& level, const Document::LDtkProject& project)
        {
            if (level.LayerInstances.has_value())
            {
                for (const Document::LDtkLayerInstance& layer : *level.LayerInstances)
                {
                    if (layer.Type == "Entities" || layer.GridSize <= 0)
                    {
                        continue;
                    }

                    return layer.GridSize;
                }
            }

            return project.DefaultGridSize > 0 ? project.DefaultGridSize : 16;
        }

        struct LevelSize
        {
            int X = 0;
            int Y = 0;
        };

        LevelSize GetLevelSize(const Document::LDtkLevel& level, int tileSize)
        {
            if (level.LayerInstances.has_value())
            {
                for (const Document::LDtkLayerInstance& layer : *level.LayerInstances)
                {
                    if (layer.Type == "Entities" || layer.CWid <= 0 || layer.CHei <= 0)
                    {
                        continue;
                    }

                    // TODO: For now, treating LDtk levels as single-grid maps.
                    //       Might need to revisit if mixed-grid levels cause issues with
                    //       per-layer cell sizes.
                    return LevelSize{layer.CWid, layer.CHei};
                }
            }

            const int x = tileSize <= 0 ? 0 : (tileSize + level.PxWid - 1) / tileSize;
            const int y = tileSize <= 0 ? 0 : (tileSize + level.PxHei - 1) / tileSize;
            return LevelSize{x, y};
        }
    }

    TilemapData Convert(const Document::LDtkLevel& level, const Document::LDtkProject& project)
    {
        const int tileSize = GetLevelGridSize(level, project);
        const LevelSize levelSize = GetLevelSize(level, tileSize);

        TilemapData data;
        data.Name = level.Identifier.empty() ? "Untitled" : level.Identifier;
        data.Width = levelSize.X;
        data.Height = levelSize.Y;
        data.TileWidth = tileSize;
        data.TileHeight = tileSize;
        // LDtk only supports orthogonal maps.
        data.Orientation = TilemapOrientation::Orthogonal;

        if (!level.BgColor.empty())
        {
            data.BackgroundColor = ParseColor(level.BgColor);
        }

        const std::unordered_map<int, int> tilesetFirstGids = ConvertTilesets(project, data);
        ConvertLayers(level, data, tilesetFirstGids);
        ConvertLevelProperties(level, data);

        return data;
    }
}
