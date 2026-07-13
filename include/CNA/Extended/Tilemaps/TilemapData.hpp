// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapData.cs: intermediate tilemap data produced by
// format parsers (Tiled/LDtk/Ogmo, ported separately) and consumed by TilemapFactory to build the
// real runtime object graph (Tilemap/TilemapLayer/TilemapTile/etc.). Upstream groups ~25 small
// sealed/abstract classes into this one file via #region blocks; ported here as one header for
// the same reason, matching the file-mirrors-namespace convention (all of them share the plain
// root `MonoGame.Extended.Tilemaps` namespace).
//
// DESIGN DEVIATION, flagged for review: every type below uses plain public mutable fields
// instead of this project's established `getXProperty()`/`setXProperty()` convention. These are
// pure, behavior-free intermediate DTOs -- C# auto-implemented properties (`{ get; set; }`, no
// validation or side effects) are semantically indistinguishable from plain fields here, and the
// getXProperty() ceremony across ~150 total properties in ~25 types would add substantial bulk
// with no behavioral benefit. This mirrors how `Size.hpp` (Phase 1) uses plain fields for a
// C# type with genuinely plain fields -- the same reasoning is extended here to C#
// auto-properties specifically because they carry no logic. This is a judgment call, not
// silently made: flagged explicitly in this session's final report for the orchestrator to
// confirm or override. If overridden, every type below would need the getXProperty()/
// setXProperty() treatment instead.
//
// Ownership: `List<T> Properties`/`Tilesets`/`Frames`/`Tiles` (of *non-polymorphic* element
// types like `TilemapPropertyData`, `TilemapTilesetEntry`, `TilemapAnimationFrameData`,
// `TilemapDecodedTile`) are plain `std::vector<T>` (by value). `List<TilemapLayerData> Layers`
// and `List<TilemapObjectData> CollisionObjects`/`Objects` (of *polymorphic, abstract-based*
// element types) are `std::vector<std::unique_ptr<T>>` -- each entry is owned solely by the
// list holding it, matching this project's established pattern for owning polymorphic
// collections (e.g. `TilemapObjectLayer::objects_`). Nullable single references to a
// non-polymorphic type (`TilemapTilesetEntry.InlineData`, `TilemapTileEntryData.Animation`) use
// `std::optional<T>` rather than `std::unique_ptr<T>`, since nothing else aliases them and the
// referenced type has no self-referential/forward-declaration need for pointer indirection.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapObjectDrawOrder.hpp"
#include "CNA/Extended/Tilemaps/TilemapOrientation.hpp"
#include "CNA/Extended/Tilemaps/TilemapPropertyType.hpp"
#include "CNA/Extended/Tilemaps/TilemapTextObject.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileFlipFlags.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector2;

    // --- Properties -----------------------------------------------------------------------

    /** @brief A single typed custom property entry (parser-intermediate; see TilemapPropertyValue for the runtime tagged-union equivalent). */
    struct TilemapPropertyData
    {
        std::string Key;
        TilemapPropertyType Type = TilemapPropertyType::String;
        /** @brief Used when Type is String or File. */
        std::string StringValue;
        /** @brief Used when Type is Int or Object. */
        int IntValue = 0;
        /** @brief Used when Type is Float. */
        float FloatValue = 0.0f;
        /** @brief Used when Type is Bool. */
        bool BoolValue = false;
        /** @brief Used when Type is Color. */
        Color ColorValue{0, 0, 0, 0};
    };

    // --- Objects ----------------------------------------------------------------------------

    /** @brief Abstract base for all tilemap object data entries (parser-intermediate; see TilemapObject for the runtime equivalent). */
    struct TilemapObjectData
    {
        virtual ~TilemapObjectData() = default;

        int Id = 0;
        std::string Name;
        std::string Class;
        float X = 0.0f;
        float Y = 0.0f;
        float Rotation = 0.0f;
        bool IsVisible = true;
        std::vector<TilemapPropertyData> Properties;
    };

    struct TilemapRectangleObjectData : TilemapObjectData
    {
        float Width = 0.0f;
        float Height = 0.0f;
    };

    struct TilemapEllipseObjectData : TilemapObjectData
    {
        float Width = 0.0f;
        float Height = 0.0f;
    };

    struct TilemapPointObjectData : TilemapObjectData
    {
    };

    struct TilemapPolygonObjectData : TilemapObjectData
    {
        std::vector<Vector2> Points;
    };

    struct TilemapPolylineObjectData : TilemapObjectData
    {
        std::vector<Vector2> Points;
    };

    struct TilemapTileObjectData : TilemapObjectData
    {
        int GlobalId = 0;
        TilemapTileFlipFlags FlipFlags = TilemapTileFlipFlags::None;
        float Width = 0.0f;
        float Height = 0.0f;
    };

    struct TilemapTextObjectData : TilemapObjectData
    {
        float Width = 0.0f;
        float Height = 0.0f;
        std::string Text;
        std::string FontFamily;
        int PixelSize = 16;
        bool WordWrap = false;
        // Fully-qualified type name: an unqualified `Color Color{...}` here would shadow the
        // `using Microsoft::Xna::Framework::Color;` alias mid-declaration (a real C++
        // restriction with no C# analogue -- C# allows a property named the same as its type).
        Microsoft::Xna::Framework::Color Color{0, 0, 0, 0};
        bool Bold = false;
        bool Italic = false;
        bool Underline = false;
        bool Strikethrough = false;
        TilemapTextObjectHorizontalAlignment HorizontalAlign = TilemapTextObjectHorizontalAlignment::Left;
        TilemapTextObjectVerticalAlignment VerticalAlign = TilemapTextObjectVerticalAlignment::Top;
    };

    // --- Per-Tile Metadata --------------------------------------------------------------------

    /** @brief A single frame in a tile animation (parser-intermediate). */
    struct TilemapAnimationFrameData
    {
        int TileId = 0;
        float Duration = 0.0f;
    };

    /** @brief Animation data for an animated tileset tile (parser-intermediate). */
    struct TilemapAnimationData
    {
        std::vector<TilemapAnimationFrameData> Frames;
    };

    /** @brief Metadata for a single tile in a tileset, including animation and collision data (parser-intermediate). */
    struct TilemapTileEntryData
    {
        int LocalId = 0;
        std::string Class;
        /** @brief Random-placement probability (1.0 = default). */
        float Probability = 1.0f;
        std::vector<TilemapPropertyData> Properties;
        /** @brief Present only if the tile is animated. */
        std::optional<TilemapAnimationData> Animation;
        std::vector<std::unique_ptr<TilemapObjectData>> CollisionObjects;
        /** @brief Only populated for image-collection tilesets where each tile has its own source image; empty for atlas-based tilesets. */
        std::string ImagePath;
    };

    // --- Tilesets -------------------------------------------------------------------------

    /** @brief Tileset data embedded directly in the map file (parser-intermediate). */
    struct TilemapTilesetData
    {
        std::string Name;
        std::string TexturePath;
        int TileWidth = 0;
        int TileHeight = 0;
        int TileCount = 0;
        int Columns = 0;
        int Spacing = 0;
        int Margin = 0;
        float DrawOffsetX = 0.0f;
        float DrawOffsetY = 0.0f;
        std::vector<TilemapPropertyData> Properties;
        std::vector<TilemapTileEntryData> Tiles;
    };

    /** @brief A tileset reference inside a TilemapData, either an external file or inline data (parser-intermediate). */
    struct TilemapTilesetEntry
    {
        int FirstGlobalId = 0;
        /** @brief When true, ExternalPath is set. When false, InlineData is set. */
        bool IsExternal = false;
        /** @brief Path to the external tileset file (e.g. .tsx). Only valid when IsExternal is true. */
        std::string ExternalPath;
        /** @brief Only valid when IsExternal is false. */
        std::optional<TilemapTilesetData> InlineData;
    };

    // --- Decoded Tiles ----------------------------------------------------------------------

    /** @brief A decoded non-empty tile entry produced during tile-layer processing (parser-intermediate). */
    struct TilemapDecodedTile
    {
        TilemapDecodedTile() = default;

        TilemapDecodedTile(std::uint16_t x, std::uint16_t y, int globalId, TilemapTileFlipFlags flipFlags)
            : X(x), Y(y), GlobalId(globalId), FlipFlags(flipFlags)
        {
        }

        std::uint16_t X = 0;
        std::uint16_t Y = 0;
        int GlobalId = 0;
        TilemapTileFlipFlags FlipFlags = TilemapTileFlipFlags::None;
    };

    // --- Layers ---------------------------------------------------------------------------

    /** @brief Abstract base for all layer data entries (parser-intermediate; see TilemapLayer for the runtime equivalent). */
    struct TilemapLayerData
    {
        virtual ~TilemapLayerData() = default;

        std::string Name;
        std::string Class;
        bool IsVisible = true;
        float Opacity = 1.0f;
        std::optional<Color> TintColor;
        float OffsetX = 0.0f;
        float OffsetY = 0.0f;
        /** @brief Parallax scroll factor X (1.0 = moves with camera). */
        float ParallaxX = 1.0f;
        /** @brief Parallax scroll factor Y (1.0 = moves with camera). */
        float ParallaxY = 1.0f;
        std::vector<TilemapPropertyData> Properties;
    };

    struct TilemapTileLayerData : TilemapLayerData
    {
        int Width = 0;
        int Height = 0;
        /** @brief Non-empty decoded tile entries for this layer. */
        std::vector<TilemapDecodedTile> Tiles;
    };

    struct TilemapObjectLayerData : TilemapLayerData
    {
        TilemapObjectDrawOrder DrawOrder = TilemapObjectDrawOrder::TopDown;
        std::vector<std::unique_ptr<TilemapObjectData>> Objects;
    };

    struct TilemapImageLayerData : TilemapLayerData
    {
        std::string TexturePath;
        bool RepeatX = false;
        bool RepeatY = false;
    };

    struct TilemapGroupLayerData : TilemapLayerData
    {
        std::vector<std::unique_ptr<TilemapLayerData>> Layers;
    };

    /** @brief Data for a layer with no visual tiles. Format-specific data is stored in TilemapLayerData::Properties. */
    struct TilemapDataLayerData : TilemapLayerData
    {
        int Width = 0;
        int Height = 0;
    };

    // --- Root Map -----------------------------------------------------------------------

    /** @brief Intermediate tilemap data produced by format parsers, consumed by TilemapFactory to build the runtime Tilemap object graph. */
    struct TilemapData
    {
        std::string Name;
        int Width = 0;
        int Height = 0;
        int TileWidth = 0;
        int TileHeight = 0;
        TilemapOrientation Orientation = TilemapOrientation::Orthogonal;
        /** @brief Stagger axis for staggered/hexagonal maps. */
        TilemapStaggerAxis StaggerAxis = TilemapStaggerAxis::X;
        /** @brief Stagger index for staggered/hexagonal maps. */
        TilemapStaggerIndex StaggerIndex = TilemapStaggerIndex::Even;
        /** @brief Hex side length in pixels for hexagonal maps. */
        int HexSideLength = 0;
        std::optional<Color> BackgroundColor;
        float ParallaxOriginX = 0.0f;
        float ParallaxOriginY = 0.0f;
        /** @brief This map's X coordinate in world space (multi-map "worlds"). */
        int WorldX = 0;
        /** @brief This map's Y coordinate in world space (multi-map "worlds"). */
        int WorldY = 0;
        /** @brief This map's depth layer in world space: positive above the surface, negative below. */
        int WorldDepth = 0;
        std::vector<TilemapPropertyData> Properties;
        std::vector<TilemapTilesetEntry> Tilesets;
        std::vector<std::unique_ptr<TilemapLayerData>> Layers;
    };
}
