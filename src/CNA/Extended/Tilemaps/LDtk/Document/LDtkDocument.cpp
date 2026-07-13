// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/LDtk/Document/LDtkDocument.hpp"

namespace CNA::Extended::Tilemaps::LDtk::Document
{
    namespace
    {
        template <typename T>
        void OptionalField(const nlohmann::ordered_json& j, const char* key, std::optional<T>& target)
        {
            if (j.contains(key) && !j.at(key).is_null())
            {
                target = j.at(key).get<T>();
            }
            else
            {
                target.reset();
            }
        }

        template <typename T>
        void DefaultedField(const nlohmann::ordered_json& j, const char* key, T& target, const T& defaultValue)
        {
            if (j.contains(key) && !j.at(key).is_null())
            {
                target = j.at(key).get<T>();
            }
            else
            {
                target = defaultValue;
            }
        }
    }

    void from_json(const nlohmann::ordered_json& j, LDtkEnumValue& value)
    {
        value.Id = j.at("id").get<std::string>();
        value.Color = j.at("color").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, LDtkEnumDefinition& value)
    {
        value.Identifier = j.at("identifier").get<std::string>();
        value.Uid = j.at("uid").get<int>();
        DefaultedField(j, "values", value.Values, {});
        DefaultedField<std::string>(j, "externalRelPath", value.ExternalRelPath, "");
    }

    void from_json(const nlohmann::ordered_json& j, LDtkIntGridValueDefinition& value)
    {
        value.Value = j.at("value").get<int>();
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "color", value.Color, "");
    }

    void from_json(const nlohmann::ordered_json& j, LDtkAutoLayerRule& value)
    {
        value.Uid = j.at("uid").get<int>();
        value.Active = j.at("active").get<bool>();
        value.Size = j.at("size").get<int>();
        DefaultedField(j, "tileRectsIds", value.TileRectsIds, {});
        DefaultedField(j, "pattern", value.Pattern, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkAutoLayerRuleGroup& value)
    {
        value.Uid = j.at("uid").get<int>();
        DefaultedField<std::string>(j, "name", value.Name, "");
        value.Active = j.at("active").get<bool>();
        DefaultedField(j, "rules", value.Rules, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkFieldDefinition& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "__type", value.Type, "");
        value.Uid = j.at("uid").get<int>();
        DefaultedField<std::string>(j, "type", value.TypeEnum, "");
        value.IsArray = j.at("isArray").get<bool>();
        value.CanBeNull = j.at("canBeNull").get<bool>();
    }

    void from_json(const nlohmann::ordered_json& j, LDtkEntityDefinition& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        value.Uid = j.at("uid").get<int>();
        value.Width = j.at("width").get<int>();
        value.Height = j.at("height").get<int>();
        DefaultedField<std::string>(j, "color", value.Color, "");
        value.PivotX = j.at("pivotX").get<float>();
        value.PivotY = j.at("pivotY").get<float>();
        DefaultedField(j, "fieldDefs", value.FieldDefs, {});
        DefaultedField(j, "tags", value.Tags, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkLayerDefinition& value)
    {
        DefaultedField<std::string>(j, "__type", value.Type, "");
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "type", value.TypeEnum, "");
        value.Uid = j.at("uid").get<int>();
        value.GridSize = j.at("gridSize").get<int>();
        value.DisplayOpacity = j.at("displayOpacity").get<float>();
        value.PxOffsetX = j.at("pxOffsetX").get<int>();
        value.PxOffsetY = j.at("pxOffsetY").get<int>();
        value.ParallaxFactorX = j.at("parallaxFactorX").get<float>();
        value.ParallaxFactorY = j.at("parallaxFactorY").get<float>();
        OptionalField(j, "tilesetDefUid", value.TilesetDefUid);
        OptionalField(j, "autoSourceLayerDefUid", value.AutoSourceLayerDefUid);
        DefaultedField(j, "intGridValues", value.IntGridValues, {});
        DefaultedField(j, "autoRuleGroups", value.AutoRuleGroups, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkTilesetDefinition& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        value.Uid = j.at("uid").get<int>();
        DefaultedField<std::string>(j, "relPath", value.RelPath, "");
        DefaultedField<std::string>(j, "embedAtlas", value.EmbedAtlas, "");
        value.PxWid = j.at("pxWid").get<int>();
        value.PxHei = j.at("pxHei").get<int>();
        value.TileGridSize = j.at("tileGridSize").get<int>();
        value.Spacing = j.at("spacing").get<int>();
        value.Padding = j.at("padding").get<int>();
        value.CWid = j.at("__cWid").get<int>();
        value.CHei = j.at("__cHei").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, LDtkDefinitions& value)
    {
        DefaultedField(j, "layers", value.Layers, {});
        DefaultedField(j, "entities", value.Entities, {});
        DefaultedField(j, "tilesets", value.Tilesets, {});
        DefaultedField(j, "enums", value.Enums, {});
        DefaultedField(j, "externalEnums", value.ExternalEnums, {});
        DefaultedField(j, "levelFields", value.LevelFields, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkFieldInstance& value)
    {
        DefaultedField<std::string>(j, "__identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "__type", value.Type, "");
        OptionalField(j, "__value", value.Value);
        value.DefUid = j.at("defUid").get<int>();
        DefaultedField(j, "realEditorValues", value.RealEditorValues, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkTileInstance& value)
    {
        DefaultedField(j, "px", value.Px, {});
        DefaultedField(j, "src", value.Src, {});
        value.FlipFlags = j.at("f").get<int>();
        value.TileId = j.at("t").get<int>();
        DefaultedField(j, "d", value.Data, {});
        DefaultedField<float>(j, "a", value.Alpha, 1.0f);
    }

    void from_json(const nlohmann::ordered_json& j, LDtkEntityInstance& value)
    {
        DefaultedField<std::string>(j, "__identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "iid", value.Iid, "");
        value.DefUid = j.at("defUid").get<int>();
        DefaultedField(j, "px", value.Px, {});
        value.Width = j.at("width").get<int>();
        value.Height = j.at("height").get<int>();
        DefaultedField(j, "__pivot", value.Pivot, {});
        DefaultedField(j, "__tags", value.Tags, {});
        DefaultedField(j, "fieldInstances", value.FieldInstances, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkNeighbourLevel& value)
    {
        DefaultedField<std::string>(j, "dir", value.Dir, "");
        DefaultedField<std::string>(j, "levelIid", value.LevelIid, "");
        OptionalField(j, "levelUid", value.LevelUid);
    }

    void from_json(const nlohmann::ordered_json& j, LDtkGridPoint& value)
    {
        value.Cx = j.at("cx").get<int>();
        value.Cy = j.at("cy").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, LDtkLayerInstance& value)
    {
        DefaultedField<std::string>(j, "__identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "__type", value.Type, "");
        value.CWid = j.at("__cWid").get<int>();
        value.CHei = j.at("__cHei").get<int>();
        value.GridSize = j.at("__gridSize").get<int>();
        value.Opacity = j.at("__opacity").get<float>();
        value.PxTotalOffsetX = j.at("__pxTotalOffsetX").get<int>();
        value.PxTotalOffsetY = j.at("__pxTotalOffsetY").get<int>();
        OptionalField(j, "__tilesetDefUid", value.TilesetDefUid);
        DefaultedField<std::string>(j, "__tilesetRelPath", value.TilesetRelPath, "");
        DefaultedField<std::string>(j, "iid", value.Iid, "");
        value.LevelId = j.at("levelId").get<int>();
        value.LayerDefUid = j.at("layerDefUid").get<int>();
        value.PxOffsetX = j.at("pxOffsetX").get<int>();
        value.PxOffsetY = j.at("pxOffsetY").get<int>();
        value.Visible = j.at("visible").get<bool>();
        value.ParallaxFactorX = j.at("__parallaxFactorX").get<float>();
        value.ParallaxFactorY = j.at("__parallaxFactorY").get<float>();
        DefaultedField(j, "gridTiles", value.GridTiles, {});
        DefaultedField(j, "autoLayerTiles", value.AutoLayerTiles, {});
        DefaultedField(j, "entityInstances", value.EntityInstances, {});
        DefaultedField(j, "intGridCsv", value.IntGridCsv, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkEntityReferenceInfos& value)
    {
        DefaultedField<std::string>(j, "entityIid", value.EntityIid, "");
        DefaultedField<std::string>(j, "layerIid", value.LayerIid, "");
        DefaultedField<std::string>(j, "levelIid", value.LevelIid, "");
        DefaultedField<std::string>(j, "worldIid", value.WorldIid, "");
    }

    void from_json(const nlohmann::ordered_json& j, LDtkTocInstanceData& value)
    {
        value.Iids = j.at("iids").get<LDtkEntityReferenceInfos>();
        value.WorldX = j.at("worldX").get<int>();
        value.WorldY = j.at("worldY").get<int>();
        value.WidPx = j.at("widPx").get<int>();
        value.HeiPx = j.at("heiPx").get<int>();
        OptionalField(j, "fields", value.Fields);
    }

    void from_json(const nlohmann::ordered_json& j, LDtkTableOfContentEntry& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField(j, "instancesData", value.InstancesData, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkLevel& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "iid", value.Iid, "");
        value.Uid = j.at("uid").get<int>();
        value.WorldX = j.at("worldX").get<int>();
        value.WorldY = j.at("worldY").get<int>();
        value.WorldDepth = j.at("worldDepth").get<int>();
        value.PxWid = j.at("pxWid").get<int>();
        value.PxHei = j.at("pxHei").get<int>();
        DefaultedField<std::string>(j, "__bgColor", value.BgColor, "");
        DefaultedField<std::string>(j, "bgColor", value.BgColorOverride, "");
        DefaultedField<std::string>(j, "bgRelPath", value.BgRelPath, "");
        DefaultedField<std::string>(j, "bgPos", value.BgPos, "");
        value.BgPivotX = j.at("bgPivotX").get<float>();
        value.BgPivotY = j.at("bgPivotY").get<float>();
        DefaultedField<std::string>(j, "externalRelPath", value.ExternalRelPath, "");
        DefaultedField(j, "fieldInstances", value.FieldInstances, {});
        // Genuinely nullable (no `= new List<>()` default upstream): unset when this level's
        // layers live in an external file instead (see ExternalRelPath).
        OptionalField(j, "layerInstances", value.LayerInstances);
        DefaultedField(j, "__neighbours", value.Neighbours, {});
    }

    void from_json(const nlohmann::ordered_json& j, LDtkWorld& value)
    {
        DefaultedField<std::string>(j, "identifier", value.Identifier, "");
        DefaultedField<std::string>(j, "iid", value.Iid, "");
        DefaultedField(j, "levels", value.Levels, {});
        value.WorldGridWidth = j.at("worldGridWidth").get<int>();
        value.WorldGridHeight = j.at("worldGridHeight").get<int>();
        DefaultedField<std::string>(j, "worldLayout", value.WorldLayout, "");
        value.DefaultLevelWidth = j.at("defaultLevelWidth").get<int>();
        value.DefaultLevelHeight = j.at("defaultLevelHeight").get<int>();
    }

    void from_json(const nlohmann::ordered_json& j, LDtkProject& value)
    {
        DefaultedField<std::string>(j, "iid", value.Iid, "");
        DefaultedField<std::string>(j, "jsonVersion", value.JsonVersion, "");
        value.AppBuildId = j.at("appBuildId").get<int>();
        value.NextUid = j.at("nextUid").get<int>();
        DefaultedField<std::string>(j, "identifierStyle", value.IdentifierStyle, "");
        DefaultedField<std::string>(j, "worldLayout", value.WorldLayout, "");
        OptionalField(j, "worldGridWidth", value.WorldGridWidth);
        OptionalField(j, "worldGridHeight", value.WorldGridHeight);
        OptionalField(j, "defaultLevelWidth", value.DefaultLevelWidth);
        OptionalField(j, "defaultLevelHeight", value.DefaultLevelHeight);
        value.DefaultGridSize = j.at("defaultGridSize").get<int>();
        value.DefaultPivotX = j.at("defaultPivotX").get<float>();
        value.DefaultPivotY = j.at("defaultPivotY").get<float>();
        DefaultedField<std::string>(j, "bgColor", value.BgColor, "");
        DefaultedField<std::string>(j, "defaultLevelBgColor", value.DefaultLevelBgColor, "");
        value.ExternalLevels = j.at("externalLevels").get<bool>();
        DefaultedField(j, "levels", value.Levels, {});
        OptionalField(j, "defs", value.Defs);
        DefaultedField(j, "worlds", value.Worlds, {});
        DefaultedField<std::string>(j, "dummyWorldIid", value.DummyWorldIid, "");
        DefaultedField(j, "toc", value.Toc, {});
    }
}
