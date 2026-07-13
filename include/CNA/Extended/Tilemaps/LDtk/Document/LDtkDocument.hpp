// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/LDtk/Document/*.cs (~21 files). Grouped into one
// header for the same reason TiledDocument.hpp groups its ~24 files: pure, behavior-free JSON
// parse targets, all sharing the `MonoGame.Extended.Tilemaps.LDtk.Document` namespace.
//
// DESIGN DEVIATION, matching TiledDocument.hpp/TilemapData.hpp's own precedent: plain public
// mutable fields instead of getXProperty()/setXProperty() -- these are C# auto-implemented
// properties with no logic, semantically indistinguishable from plain fields.
//
// Unlike Tiled's XML (deserialized via reflection-driven XmlSerializer, worked around with a
// hand-written DOM walk), LDtk is plain JSON with every property explicitly
// `[JsonPropertyName(...)]`-annotated -- this maps directly onto nlohmann's ADL `from_json`
// customization point (this project's established JSON-DTO-it-owns pattern, see
// Content/TexturePacker/TexturePackerFileContent.hpp) with no reflection gap at all. Since these
// are plain-public-field types (not the private-field-plus-friend style TexturePacker uses),
// `from_json` free functions are declared without `friend` -- there is nothing private to grant
// access to.
//
// `JsonElement?` (LDtkFieldInstance::Value, LDtkTocInstanceData::Fields) has no fixed C++ type --
// its shape depends on the field's LDtk type (int/float/bool/color/entity-ref/string/array), only
// interpreted downstream in Converters::Convert. Ported as
// `std::optional<nlohmann::ordered_json>`, the direct structural analogue: a lazily-typed raw JSON
// value, exactly matching what `JsonElement` is upstream. `List<object> RealEditorValues` (LDtk
// editor-only data, confirmed via grep to have zero consumers anywhere in the parser/converter)
// is ported the same way, as `std::vector<nlohmann::ordered_json>`, for structural completeness
// rather than dropped.
#pragma once

#include "nlohmann/json.hpp"

#include <optional>
#include <string>
#include <vector>

namespace CNA::Extended::Tilemaps::LDtk::Document
{
    // --- Enums ------------------------------------------------------------------------------

    struct LDtkEnumValue
    {
        std::string Id;
        int Color = 0;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkEnumValue& value);

    struct LDtkEnumDefinition
    {
        std::string Identifier;
        int Uid = 0;
        std::vector<LDtkEnumValue> Values;
        std::string ExternalRelPath;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkEnumDefinition& value);

    // --- IntGrid ------------------------------------------------------------------------------

    struct LDtkIntGridValueDefinition
    {
        int Value = 0;
        std::string Identifier;
        std::string Color;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkIntGridValueDefinition& value);

    // --- Auto-Layer Rules -----------------------------------------------------------------------

    struct LDtkAutoLayerRule
    {
        int Uid = 0;
        bool Active = false;
        int Size = 0;
        std::vector<std::vector<int>> TileRectsIds;
        std::vector<int> Pattern;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkAutoLayerRule& value);

    struct LDtkAutoLayerRuleGroup
    {
        int Uid = 0;
        std::string Name;
        bool Active = false;
        std::vector<LDtkAutoLayerRule> Rules;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkAutoLayerRuleGroup& value);

    // --- Fields -------------------------------------------------------------------------------

    struct LDtkFieldDefinition
    {
        std::string Identifier;
        std::string Type;
        int Uid = 0;
        std::string TypeEnum;
        bool IsArray = false;
        bool CanBeNull = false;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkFieldDefinition& value);

    // --- Entities -----------------------------------------------------------------------------

    struct LDtkEntityDefinition
    {
        std::string Identifier;
        int Uid = 0;
        int Width = 0;
        int Height = 0;
        std::string Color;
        float PivotX = 0.0f;
        float PivotY = 0.0f;
        std::vector<LDtkFieldDefinition> FieldDefs;
        std::vector<std::string> Tags;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkEntityDefinition& value);

    // --- Layer Definitions ----------------------------------------------------------------------

    struct LDtkLayerDefinition
    {
        std::string Type;
        std::string Identifier;
        std::string TypeEnum;
        int Uid = 0;
        int GridSize = 0;
        float DisplayOpacity = 0.0f;
        int PxOffsetX = 0;
        int PxOffsetY = 0;
        float ParallaxFactorX = 0.0f;
        float ParallaxFactorY = 0.0f;
        std::optional<int> TilesetDefUid;
        std::optional<int> AutoSourceLayerDefUid;
        std::vector<LDtkIntGridValueDefinition> IntGridValues;
        std::vector<LDtkAutoLayerRuleGroup> AutoRuleGroups;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkLayerDefinition& value);

    // --- Tilesets -----------------------------------------------------------------------------

    struct LDtkTilesetDefinition
    {
        std::string Identifier;
        int Uid = 0;
        std::string RelPath;
        std::string EmbedAtlas;
        int PxWid = 0;
        int PxHei = 0;
        int TileGridSize = 0;
        int Spacing = 0;
        int Padding = 0;
        int CWid = 0;
        int CHei = 0;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkTilesetDefinition& value);

    // --- Definitions (root) ---------------------------------------------------------------------

    struct LDtkDefinitions
    {
        std::vector<LDtkLayerDefinition> Layers;
        std::vector<LDtkEntityDefinition> Entities;
        std::vector<LDtkTilesetDefinition> Tilesets;
        std::vector<LDtkEnumDefinition> Enums;
        std::vector<LDtkEnumDefinition> ExternalEnums;
        std::vector<LDtkFieldDefinition> LevelFields;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkDefinitions& value);

    // --- Field Instances ------------------------------------------------------------------------

    struct LDtkFieldInstance
    {
        std::string Identifier;
        std::string Type;
        std::optional<nlohmann::ordered_json> Value;
        int DefUid = 0;
        std::vector<nlohmann::ordered_json> RealEditorValues;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkFieldInstance& value);

    // --- Tile Instances -------------------------------------------------------------------------

    struct LDtkTileInstance
    {
        std::vector<int> Px;
        std::vector<int> Src;
        int FlipFlags = 0;
        int TileId = 0;
        std::vector<int> Data;
        float Alpha = 1.0f;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkTileInstance& value);

    // --- Entity Instances -----------------------------------------------------------------------

    struct LDtkEntityInstance
    {
        std::string Identifier;
        std::string Iid;
        int DefUid = 0;
        std::vector<int> Px;
        int Width = 0;
        int Height = 0;
        std::vector<float> Pivot;
        std::vector<std::string> Tags;
        std::vector<LDtkFieldInstance> FieldInstances;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkEntityInstance& value);

    // --- Neighbours -----------------------------------------------------------------------------

    struct LDtkNeighbourLevel
    {
        std::string Dir;
        std::string LevelIid;
        std::optional<int> LevelUid;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkNeighbourLevel& value);

    // --- Grid Points ----------------------------------------------------------------------------

    struct LDtkGridPoint
    {
        int Cx = 0;
        int Cy = 0;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkGridPoint& value);

    // --- Layer Instances ------------------------------------------------------------------------

    struct LDtkLayerInstance
    {
        std::string Identifier;
        std::string Type;
        int CWid = 0;
        int CHei = 0;
        int GridSize = 0;
        float Opacity = 0.0f;
        int PxTotalOffsetX = 0;
        int PxTotalOffsetY = 0;
        std::optional<int> TilesetDefUid;
        std::string TilesetRelPath;
        std::string Iid;
        int LevelId = 0;
        int LayerDefUid = 0;
        int PxOffsetX = 0;
        int PxOffsetY = 0;
        bool Visible = false;
        float ParallaxFactorX = 0.0f;
        float ParallaxFactorY = 0.0f;
        std::vector<LDtkTileInstance> GridTiles;
        std::vector<LDtkTileInstance> AutoLayerTiles;
        std::vector<LDtkEntityInstance> EntityInstances;
        std::vector<int> IntGridCsv;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkLayerInstance& value);

    // --- Entity References / Table of Contents ---------------------------------------------------

    /** @brief Entity reference information in LDtk, identifying an entity instance by its IIDs. */
    struct LDtkEntityReferenceInfos
    {
        std::string EntityIid;
        std::string LayerIid;
        std::string LevelIid;
        std::string WorldIid;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkEntityReferenceInfos& value);

    /** @brief Instance data for a table-of-contents entry in LDtk. */
    struct LDtkTocInstanceData
    {
        LDtkEntityReferenceInfos Iids;
        int WorldX = 0;
        int WorldY = 0;
        int WidPx = 0;
        int HeiPx = 0;
        std::optional<nlohmann::ordered_json> Fields;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkTocInstanceData& value);

    struct LDtkTableOfContentEntry
    {
        std::string Identifier;
        std::vector<LDtkTocInstanceData> InstancesData;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkTableOfContentEntry& value);

    // --- Levels ---------------------------------------------------------------------------------

    struct LDtkLevel
    {
        std::string Identifier;
        std::string Iid;
        int Uid = 0;
        int WorldX = 0;
        int WorldY = 0;
        int WorldDepth = 0;
        int PxWid = 0;
        int PxHei = 0;
        std::string BgColor;
        std::string BgColorOverride;
        std::string BgRelPath;
        std::string BgPos;
        float BgPivotX = 0.0f;
        float BgPivotY = 0.0f;
        std::string ExternalRelPath;
        std::vector<LDtkFieldInstance> FieldInstances;
        /** @brief Unset (not merely empty) when this level's layers live in an external file (see ExternalRelPath). */
        std::optional<std::vector<LDtkLayerInstance>> LayerInstances;
        std::vector<LDtkNeighbourLevel> Neighbours;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkLevel& value);

    // --- Worlds ---------------------------------------------------------------------------------

    struct LDtkWorld
    {
        std::string Identifier;
        std::string Iid;
        std::vector<LDtkLevel> Levels;
        int WorldGridWidth = 0;
        int WorldGridHeight = 0;
        std::string WorldLayout;
        int DefaultLevelWidth = 0;
        int DefaultLevelHeight = 0;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkWorld& value);

    // --- Project (root) -------------------------------------------------------------------------

    struct LDtkProject
    {
        std::string Iid;
        std::string JsonVersion;
        int AppBuildId = 0;
        int NextUid = 0;
        std::string IdentifierStyle;
        std::string WorldLayout;
        std::optional<int> WorldGridWidth;
        std::optional<int> WorldGridHeight;
        std::optional<int> DefaultLevelWidth;
        std::optional<int> DefaultLevelHeight;
        int DefaultGridSize = 0;
        float DefaultPivotX = 0.0f;
        float DefaultPivotY = 0.0f;
        std::string BgColor;
        std::string DefaultLevelBgColor;
        bool ExternalLevels = false;
        std::vector<LDtkLevel> Levels;
        std::optional<LDtkDefinitions> Defs;
        std::vector<LDtkWorld> Worlds;
        std::string DummyWorldIid;
        std::vector<LDtkTableOfContentEntry> Toc;
    };

    void from_json(const nlohmann::ordered_json& j, LDtkProject& value);
}
