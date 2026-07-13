// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tiled/Document/*.cs (~24 files). Grouped into one
// header for the same reason TilemapData.hpp groups ~25 related DTOs into one file: these are
// pure, behavior-free parse targets, all sharing the `MonoGame.Extended.Tilemaps.Tiled.Document`
// namespace.
//
// DESIGN DEVIATION, matching TilemapData.hpp's own precedent: plain public mutable fields
// instead of getXProperty()/setXProperty() -- these are C# auto-implemented properties with no
// logic, semantically indistinguishable from plain fields.
//
// REFLECTION GAP: upstream deserializes this whole tree via `System.Xml.Serialization.XmlSerializer`
// driven by [XmlAttribute]/[XmlElement]/[XmlText] attributes on every property -- a genuine
// reflection dependency with no C++ equivalent. TiledTmxParser.cpp works around this not by
// dropping the two-stage (raw XML -> Document model -> TilemapData) architecture, but by hand-
// writing the XML-to-Document-model walk that XmlSerializer would otherwise generate via
// reflection, using the already-ported System::Xml DOM (XmlDocument/XmlElement) directly. The
// Document model types themselves are unaffected by this -- they're still exactly what
// TiledTilemapDataConverter.cpp consumes, faithfully matching upstream's architecture.
//
// [XmlIgnore] `Texture2D Texture` fields (runtime-loaded, not part of XML parsing) are ported as
// non-owning `Texture2D*` (external-GPU-resource convention, matching Tilemap::ownedTextures_'s
// established pattern) -- though in practice TilemapFactory/TiledTilemapDataConverter's own
// texture-loading path (via TexturePath strings resolved by TilemapFactory) is what's actually
// used; these fields are carried for structural fidelity, not currently populated by the parser.
//
// TiledTilesetRefXml : TiledTilesetXml and the Tiled{Tile,Object,Image,Group}LayerXml : TiledLayerXml
// inheritance hierarchies are preserved (upstream's own polymorphic XmlElement-attribute-driven
// deserialization relies on exactly this hierarchy for its `List<TiledLayerXml>` covariant
// storage), with virtual destructors added for safe dynamic_cast dispatch in
// TiledTilemapDataConverter's format-detection switches (C# `is`/pattern-matching's C++
// equivalent).
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Tilemaps::Tiled::Document
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    // --- Properties -------------------------------------------------------------------------

    struct TiledPropertyXml
    {
        std::string Name;
        std::string Type;
        std::string Value;
    };

    struct TiledPropertiesXml
    {
        std::vector<TiledPropertyXml> Properties;
    };

    // --- Shapes / markers ---------------------------------------------------------------------

    struct TiledEllipseXml
    {
    };

    struct TiledPointXml
    {
    };

    struct TiledPolygonXml
    {
        std::string Points;
    };

    struct TiledPolylineXml
    {
        std::string Points;
    };

    struct TiledTextXml
    {
        std::string FontFamily;
        int PixelSize = 16;
        int Wrap = 0;
        std::string Color = "#000000";
        int Bold = 0;
        int Italic = 0;
        int Underline = 0;
        int Strikeout = 0;
        int Kerning = 1;
        std::string HAlign = "left";
        std::string VAlign = "top";
        std::string Value;
    };

    // --- Objects ------------------------------------------------------------------------------

    struct TiledObjectXml
    {
        int Id = 0;
        std::string Name;
        std::string Type;
        std::string Class;
        float X = 0.0f;
        float Y = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float Rotation = 0.0f;
        std::uint32_t Gid = 0;
        int Visible = 1;
        std::unique_ptr<TiledPropertiesXml> Properties;
        std::unique_ptr<TiledEllipseXml> Ellipse;
        std::unique_ptr<TiledPointXml> Point;
        std::unique_ptr<TiledPolygonXml> Polygon;
        std::unique_ptr<TiledPolylineXml> Polyline;
        std::unique_ptr<TiledTextXml> Text;
    };

    struct TiledObjectGroupXml
    {
        std::string DrawOrder;
        std::vector<std::unique_ptr<TiledObjectXml>> Objects;
    };

    // --- Tile animation / offset / grid ---------------------------------------------------------

    struct TiledAnimationFrameXml
    {
        int TileId = 0;
        int Duration = 0;
    };

    struct TiledAnimationXml
    {
        std::vector<TiledAnimationFrameXml> Frames;
    };

    struct TiledTileOffsetXml
    {
        int X = 0;
        int Y = 0;
    };

    struct TiledGridXml
    {
        std::string Orientation;
        int Width = 0;
        int Height = 0;
    };

    struct TiledImageXml
    {
        std::string Source;
        int Width = 0;
        int Height = 0;
        std::string Trans;
        Texture2D* Texture = nullptr;
    };

    // --- Tileset --------------------------------------------------------------------------------

    struct TiledTileXml
    {
        int Id = 0;
        std::string Type;
        std::string Class;
        float Probability = 0.0f;
        std::unique_ptr<TiledPropertiesXml> Properties;
        std::unique_ptr<TiledImageXml> Image;
        std::unique_ptr<TiledObjectGroupXml> ObjectGroup;
        std::unique_ptr<TiledAnimationXml> Animation;
    };

    /** @brief Represents a tileset element from a Tiled TMX or TSX file. */
    struct TiledTilesetXml
    {
        virtual ~TiledTilesetXml() = default;

        int FirstGlobalId = 0;
        std::string Name;
        int TileWidth = 0;
        int TileHeight = 0;
        int TileCount = 0;
        int Columns = 0;
        int Spacing = 0;
        int Margin = 0;
        std::string ObjectAlignment;
        std::unique_ptr<TiledTileOffsetXml> TileOffset;
        std::unique_ptr<TiledGridXml> Grid;
        std::unique_ptr<TiledImageXml> Image;
        std::vector<std::unique_ptr<TiledTileXml>> Tiles;
        std::unique_ptr<TiledPropertiesXml> Properties;
        Texture2D* Texture = nullptr;
    };

    /** @brief A tileset reference in a Tiled TMX file (can be inline or external). */
    struct TiledTilesetRefXml : TiledTilesetXml
    {
        std::string Source;
        /** @brief Populated during parsing when Source is specified (loaded from an external TSX file). */
        std::unique_ptr<TiledTilesetXml> TilesetData;
    };

    // --- Layers -------------------------------------------------------------------------------

    /** @brief Base for all Tiled layer types. */
    struct TiledLayerXml
    {
        virtual ~TiledLayerXml() = default;

        int Id = 0;
        std::string Name;
        std::string Class;
        float OffsetX = 0.0f;
        float OffsetY = 0.0f;
        float ParallaxX = 1.0f;
        float ParallaxY = 1.0f;
        float Opacity = 1.0f;
        int Visible = 1;
        std::string TintColor;
        std::unique_ptr<TiledPropertiesXml> Properties;
    };

    struct TiledDataTileXml
    {
        std::uint32_t Gid = 0;
    };

    struct TiledChunkXml
    {
        int X = 0;
        int Y = 0;
        int Width = 0;
        int Height = 0;
        std::string Value;
    };

    struct TiledTileLayerDataXml
    {
        std::string Encoding;
        std::string Compression;
        std::string Value;
        std::vector<TiledDataTileXml> Tiles;
        std::vector<TiledChunkXml> Chunks;
    };

    struct TiledTileLayerXml : TiledLayerXml
    {
        int Width = 0;
        int Height = 0;
        std::unique_ptr<TiledTileLayerDataXml> Data;
    };

    struct TiledObjectLayerXml : TiledLayerXml
    {
        std::string Color;
        std::string DrawOrder;
        std::vector<std::unique_ptr<TiledObjectXml>> Objects;
    };

    struct TiledImageLayerXml : TiledLayerXml
    {
        int RepeatX = 0;
        int RepeatY = 0;
        std::unique_ptr<TiledImageXml> Image;
        Texture2D* Texture = nullptr;
    };

    struct TiledGroupLayerXml : TiledLayerXml
    {
        std::vector<std::unique_ptr<TiledLayerXml>> Layers;
    };

    // --- Root map -----------------------------------------------------------------------------

    /** @brief Represents the root map element from a Tiled TMX file. */
    struct TiledMapXml
    {
        std::string Version;
        std::string TiledVersion;
        std::string Orientation;
        std::string RenderOrder;
        int Width = 0;
        int Height = 0;
        int TileWidth = 0;
        int TileHeight = 0;
        int Infinite = 0;
        std::string BackgroundColor;
        float ParallaxOriginX = 0.0f;
        float ParallaxOriginY = 0.0f;
        int HexSideLength = 0;
        std::string StaggerAxis;
        std::string StaggerIndex;
        int NextLayerId = 0;
        int NextObjectId = 0;
        std::unique_ptr<TiledPropertiesXml> Properties;
        std::vector<std::unique_ptr<TiledTilesetRefXml>> Tilesets;
        std::vector<std::unique_ptr<TiledLayerXml>> Layers;
    };
}
