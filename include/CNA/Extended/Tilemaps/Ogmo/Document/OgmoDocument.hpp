// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Ogmo/Document/*.cs (~16 files). Grouped into one
// header for the same reason TiledDocument.hpp groups its own ~24 files: these are pure,
// behavior-free parse targets, all sharing the `MonoGame.Extended.Tilemaps.Ogmo.Document`
// namespace.
//
// DESIGN DEVIATION, matching TilemapData.hpp/TiledDocument.hpp's own precedent: plain public
// mutable fields instead of getXProperty()/setXProperty() -- these are C# auto-implemented
// properties with no logic, semantically indistinguishable from plain fields.
//
// Unlike Tiled (XML, reflection-driven XmlSerializer), Ogmo is JSON -- deserialized here via
// `sharp-runtime`'s `System::Text::Json` (nlohmann `from_json` ADL customization point), matching
// this project's established convention for JSON-backed types this port owns the definition of
// (see `Content/TexturePacker/TexturePackerFileContent.hpp`). No reflection gap exists here: C#'s
// own `System.Text.Json` deserialization for these types is itself driven by source-generated/
// reflection-based property binding, but the actual *shape* (a tree of named fields) has a direct,
// non-reflective C++ translation via explicit `from_json` functions, one per type.
//
// `Dictionary<string, object> Values` (untyped custom values, see `OgmoLevel`/`OgmoEntity`/
// `OgmoDecal`) is ported as a raw `nlohmann::ordered_json` object node, not a C++ map -- this is
// actually a *more direct* translation than upstream's own `System.Text.Json` deserialization
// target (`Dictionary<string, object>`, where each value becomes a boxed `JsonElement` in
// practice), since `Converters::ConvertCustomValue` needs exactly the same "inspect the raw JSON
// value's kind" logic C#'s `JsonElement.ValueKind` switch performs -- nlohmann's `is_string()`/
// `is_boolean()`/`is_number_integer()`/etc. give that directly, with no intermediate type needed.
//
// `OgmoLayerData` (abstract polymorphic base, `[JsonConverter(typeof(OgmoLayerDataConverter))]`
// upstream) is NOT reflection-based -- `OgmoLayerDataConverter.Read` is a plain field-presence
// check (does the JSON object have a "tileset"/"data"/"data2D" key? -> OgmoTileLayerData; a
// "grid"/"grid2D" key? -> OgmoGridLayerData; etc.), translated directly as the free function
// `ParseOgmoLayerData` below returning `std::unique_ptr<OgmoLayerData>` -- the C++ equivalent of
// upstream's dispatch, not a `nlohmann::adl_serializer` specialization (which has no natural way
// to return a polymorphic base pointer), matching this project's precedent of using an explicit
// factory function for polymorphic JSON dispatch (see `ParticleEffectSerializer.cpp`'s
// upstream-mirroring manual type-name dispatch, itself confirmed to have no reflection either).
//
// `List<Point?> DataCoords` / `List<List<Point?>> DataCoords2D` (`OgmoTileCoordListConverter`/
// `OgmoTileCoordList2DConverter` upstream: `[[tileX, tileY], [-1], ...]`, a single-element array
// with a negative value signalling an empty cell) ported as `std::vector<std::optional<Point>>`/
// `std::vector<std::vector<std::optional<Point>>>`, with the exact same "count >= 2 elements is a
// valid pair, otherwise empty" parsing rule preserved in `ParseCoordPair` below.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"
#include "nlohmann/json.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace CNA::Extended::Tilemaps::Ogmo::Document
{
    using Microsoft::Xna::Framework::Point;

    // --- Small value types --------------------------------------------------------------------

    struct OgmoVector2
    {
        float X = 0.0f;
        float Y = 0.0f;
    };

    struct OgmoShape
    {
        std::string Label;
        std::vector<OgmoVector2> Points;
    };

    struct OgmoValueTemplate
    {
        std::string Name;
        std::string Definition;
        /** @brief Raw JSON default value; its shape depends on Definition (String/Integer/Float/Boolean/Color/Enum/...). */
        nlohmann::ordered_json Defaults;
        bool Bounded = false;
        std::optional<float> Min;
        std::optional<float> Max;
        std::optional<int> MaxLength;
        bool TrimWhitespace = false;
        std::vector<std::string> Choices;
        bool IncludeAlpha = false;
    };

    struct OgmoTilesetTemplate
    {
        std::string Label;
        std::string Path;
        std::string Image;
        int TileWidth = 0;
        int TileHeight = 0;
        int TileSeparationX = 0;
        int TileSeparationY = 0;
    };

    struct OgmoEntityTemplate
    {
        std::string Name;
        std::string ExportID;
        int Limit = 0;
        OgmoVector2 Size;
        OgmoVector2 Origin;
        bool OriginAnchored = false;
        OgmoShape Shape;
        std::string Color;
        bool TileX = false;
        bool TileY = false;
        OgmoVector2 TileSize;
        bool ResizeableX = false;
        bool ResizeableY = false;
        bool Rotatable = false;
        float RotationDegrees = 0.0f;
        bool CanFlipX = false;
        bool CanFlipY = false;
        bool CanSetColor = false;
        bool HasNodes = false;
        int NodeLimit = 0;
        int NodeDisplay = 0;
        bool NodeGhost = false;
        std::vector<std::string> Tags;
        std::vector<OgmoValueTemplate> Values;
        std::string Texture;
        std::string TextureImage;
    };

    struct OgmoLayerTemplate
    {
        std::string Name;
        std::string Definition;
        OgmoVector2 GridSize;
        std::string ExportID;
        int ExportMode = 0;
        int ArrayMode = 0;
        std::string DefaultTileset;
        /** @brief Raw JSON object (character -> tileset/tile name); stored raw since Converters::ConvertGridLayer re-serializes it verbatim into an "Ogmo_GridLegend" property. */
        nlohmann::ordered_json Legend;
        std::vector<std::string> RequiredTags;
        std::vector<std::string> ExcludedTags;
        std::string Folder;
        bool IncludeImageSequence = false;
        bool Scaleable = false;
        bool Rotatable = false;
        std::vector<OgmoValueTemplate> Values;
    };

    struct OgmoProject
    {
        std::string Name;
        std::string OgmoVersion;
        std::vector<std::string> LevelPaths;
        std::string BackgroundColor;
        std::string GridColor;
        bool AnglesRadians = false;
        int DirectoryDepth = 0;
        std::optional<OgmoVector2> LayerGridDefaultSize;
        std::optional<OgmoVector2> LevelDefaultSize;
        std::optional<OgmoVector2> LevelMinSize;
        std::optional<OgmoVector2> LevelMaxSize;
        std::vector<OgmoValueTemplate> LevelValues;
        std::string DefaultExportMode;
        bool CompactExport = false;
        std::vector<std::string> EntityTags;
        std::vector<OgmoLayerTemplate> Layers;
        std::vector<OgmoEntityTemplate> Entities;
        std::vector<OgmoTilesetTemplate> Tilesets;
    };

    // --- Level content --------------------------------------------------------------------

    struct OgmoEntity
    {
        std::string Name;
        int Id = 0;
        std::string ExportID;
        float X = 0.0f;
        float Y = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float OriginX = 0.0f;
        float OriginY = 0.0f;
        float Rotation = 0.0f;
        bool FlippedX = false;
        bool FlippedY = false;
        std::vector<OgmoVector2> Nodes;
        nlohmann::ordered_json Values;
    };

    struct OgmoDecal
    {
        float X = 0.0f;
        float Y = 0.0f;
        std::string Texture;
        float Rotation = 0.0f;
        float ScaleX = 0.0f;
        float ScaleY = 0.0f;
        nlohmann::ordered_json Values;
    };

    /** @brief Abstract base for all Ogmo layer-data entries. Concrete subtype is chosen by ParseOgmoLayerData based on which JSON keys are present. */
    struct OgmoLayerData
    {
        virtual ~OgmoLayerData() = default;

        std::string Name;
        std::string ExportID;
        float OffsetX = 0.0f;
        float OffsetY = 0.0f;
        int GridCellWidth = 0;
        int GridCellHeight = 0;
        int GridCellsX = 0;
        int GridCellsY = 0;
    };

    struct OgmoTileLayerData : OgmoLayerData
    {
        std::string Tileset;
        std::vector<int> Data;
        std::vector<std::vector<int>> Data2D;
        std::vector<std::optional<Point>> DataCoords;
        std::vector<std::vector<std::optional<Point>>> DataCoords2D;
        int ExportMode = 0;
        int ArrayMode = 0;
    };

    struct OgmoGridLayerData : OgmoLayerData
    {
        std::vector<std::string> Grid;
        std::vector<std::vector<std::string>> Grid2D;
        int ArrayMode = 0;
    };

    struct OgmoEntityLayerData : OgmoLayerData
    {
        std::vector<OgmoEntity> Entities;
    };

    struct OgmoDecalLayerData : OgmoLayerData
    {
        std::vector<OgmoDecal> Decals;
        std::string Folder;
    };

    struct OgmoLevel
    {
        std::string OgmoVersion;
        float Width = 0.0f;
        float Height = 0.0f;
        float OffsetX = 0.0f;
        float OffsetY = 0.0f;
        nlohmann::ordered_json Values;
        std::vector<std::unique_ptr<OgmoLayerData>> Layers;
    };

    // --- Deserialization ------------------------------------------------------------------

    void from_json(const nlohmann::ordered_json& j, OgmoVector2& value);
    void from_json(const nlohmann::ordered_json& j, OgmoShape& value);
    void from_json(const nlohmann::ordered_json& j, OgmoValueTemplate& value);
    void from_json(const nlohmann::ordered_json& j, OgmoTilesetTemplate& value);
    void from_json(const nlohmann::ordered_json& j, OgmoEntityTemplate& value);
    void from_json(const nlohmann::ordered_json& j, OgmoLayerTemplate& value);
    void from_json(const nlohmann::ordered_json& j, OgmoProject& value);
    void from_json(const nlohmann::ordered_json& j, OgmoEntity& value);
    void from_json(const nlohmann::ordered_json& j, OgmoDecal& value);
    void from_json(const nlohmann::ordered_json& j, OgmoLevel& value);

    /** @brief Parses a [[tileX,tileY],[-1],...]-style coordinate entry, matching upstream OgmoTileCoordListConverter.ReadCoordPair exactly. */
    [[nodiscard]] std::optional<Point> ParseCoordPair(const nlohmann::ordered_json& pairArray);

    /**
     * @brief Parses a single OgmoLayerData JSON object, dispatching to the correct concrete
     * subtype based on which fields are present, matching upstream OgmoLayerDataConverter.Read.
     */
    [[nodiscard]] std::unique_ptr<OgmoLayerData> ParseOgmoLayerData(const nlohmann::ordered_json& j);
}
