// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/Tilemap.cs. `Properties`/`Tilesets`/`Layers` are
// owned by value (constructed in the constructor, matching upstream exactly), since
// `TilemapTilesetCollection`/`TilemapLayerCollection` are themselves the canonical owners of
// their contents (see those types' own header comments). `internal set` on HexSideLength/
// StaggerAxis/StaggerIndex is kept as an ordinary public setter, matching this project's
// established internal-visibility convention (C++ has no assembly-boundary equivalent).
//
// `ownedTextures_`/`AddOwnedTexture` (new, no direct upstream field): in C#, each texture
// `TilemapFactory` loads (`Texture2D.FromStream(...)`) is a GC-managed object kept alive purely
// by whichever tileset/tile-data/image-layer field references it -- no separate "owner" is
// needed. In C++, `Texture2D::FromStream` returns *by value*, and `TilemapTileset`/
// `TilemapTileData`/`TilemapImageLayer` all (correctly, matching this project's established
// externally-owned-GPU-resource convention) hold only a non-owning `Texture2D*` -- so something
// has to actually own each loaded texture for as long as the `Tilemap` referencing it is alive.
// `Tilemap` itself is that owner (mirroring `BitmapFont::pageTextures_`'s identical problem and
// solution); `TilemapFactory::Build` is the only expected caller of `AddOwnedTexture`, kept
// public (not a `friend`-restricted API) since free functions can't be granted class-private
// access, matching this project's established "internal visibility kept public, documented"
// convention.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapLayerCollection.hpp"
#include "CNA/Extended/Tilemaps/TilemapOrientation.hpp"
#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"
#include "CNA/Extended/Tilemaps/TilemapTilesetCollection.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @brief The runtime tile/layer/tileset data model for a loaded map, independent of which
     * source format it came from (Tiled TMX/JSON, LDtk JSON, or Ogmo JSON). Every format-specific
     * parser converges on this same representation, so rendering, coordinate conversion, and
     * gameplay code written against Tilemap never needs to know or care which format a given map
     * was authored in.
     *
     * A Tilemap is data only -- it owns its layers, tilesets, and any textures it loaded, but has
     * no drawing logic of its own. Load one with a format parser (e.g.
     * Tiled::TiledTmxParser::ParseFromFile) or by hand-assembling layers/tilesets, then hand it to
     * one of the Rendering::* renderers to draw it.
     *
     * @see CNA::Extended::Tilemaps::Build (TilemapFactory.hpp) for the lower-level entry point
     * format parsers use internally to turn a parsed TilemapData into a Tilemap.
     * @see Tiled::TiledTmxParser, LDtk::LDtkWorld, Ogmo::OgmoProject for the format-specific
     * parsers that produce a Tilemap directly.
     * @see Rendering::TilemapSpriteBatchRenderer, Rendering::TilemapRenderer, and their World-*
     * counterparts for drawing a loaded Tilemap.
     */
    class Tilemap
    {
    public:
        /** @brief Creates an empty tilemap with the given dimensions and orientation. */
        Tilemap(std::string name, int width, int height, int tileWidth, int tileHeight, TilemapOrientation orientation);

        /** @brief Declared (not defaulted) out-of-line: `ownedTextures_`'s `unique_ptr<Texture2D>` destructor needs `Texture2D` complete, which this header only forward-declares. */
        ~Tilemap();
        Tilemap(Tilemap&&) noexcept = default;
        Tilemap& operator=(Tilemap&&) noexcept = default;
        Tilemap(const Tilemap&) = delete;
        Tilemap& operator=(const Tilemap&) = delete;

        [[nodiscard]] const std::string& getNameProperty() const { return name_; }
        void setNameProperty(std::string value) { name_ = std::move(value); }

        [[nodiscard]] int getWidthProperty() const { return width_; }
        [[nodiscard]] int getHeightProperty() const { return height_; }
        [[nodiscard]] int getTileWidthProperty() const { return tileWidth_; }
        [[nodiscard]] int getTileHeightProperty() const { return tileHeight_; }
        [[nodiscard]] TilemapOrientation getOrientationProperty() const { return orientation_; }

        /** @brief Gets/sets the hex tile side length in pixels. Only meaningful for hexagonal maps. */
        [[nodiscard]] int getHexSideLengthProperty() const { return hexSideLength_; }
        void setHexSideLengthProperty(int value) { hexSideLength_ = value; }

        /** @brief Gets/sets the stagger axis. Only meaningful for staggered/hexagonal maps. */
        [[nodiscard]] TilemapStaggerAxis getStaggerAxisProperty() const { return staggerAxis_; }
        void setStaggerAxisProperty(TilemapStaggerAxis value) { staggerAxis_ = value; }

        /** @brief Gets/sets the stagger index. Only meaningful for staggered/hexagonal maps. */
        [[nodiscard]] TilemapStaggerIndex getStaggerIndexProperty() const { return staggerIndex_; }
        void setStaggerIndexProperty(TilemapStaggerIndex value) { staggerIndex_ = value; }

        [[nodiscard]] const std::optional<Color>& getBackgroundColorProperty() const { return backgroundColor_; }
        void setBackgroundColorProperty(const std::optional<Color>& value) { backgroundColor_ = value; }

        /** @brief Gets/sets the parallax origin in world pixels. */
        [[nodiscard]] const Vector2& getParallaxOriginProperty() const { return parallaxOrigin_; }
        void setParallaxOriginProperty(const Vector2& value) { parallaxOrigin_ = value; }

        /** @brief Gets/sets the position of this map in world space. */
        [[nodiscard]] const Vector2& getWorldPositionProperty() const { return worldPosition_; }
        void setWorldPositionProperty(const Vector2& value) { worldPosition_ = value; }

        /** @brief Gets/sets the depth layer of this map: positive above the surface, negative below. */
        [[nodiscard]] int getWorldDepthProperty() const { return worldDepth_; }
        void setWorldDepthProperty(int value) { worldDepth_ = value; }

        /** @brief Gets the custom properties of the tilemap. */
        [[nodiscard]] TilemapProperties& getPropertiesProperty() { return properties_; }
        [[nodiscard]] const TilemapProperties& getPropertiesProperty() const { return properties_; }

        /** @brief Gets the collection of tilesets used by this tilemap. */
        [[nodiscard]] TilemapTilesetCollection& getTilesetsProperty() { return tilesets_; }
        [[nodiscard]] const TilemapTilesetCollection& getTilesetsProperty() const { return tilesets_; }

        /** @brief Gets the collection of layers in this tilemap. */
        [[nodiscard]] TilemapLayerCollection& getLayersProperty() { return layers_; }
        [[nodiscard]] const TilemapLayerCollection& getLayersProperty() const { return layers_; }

        /** @brief Gets the world-space bounds of the tilemap. */
        [[nodiscard]] Rectangle getWorldBoundsProperty() const;

        /** @brief Converts tile coordinates (@p x, @p y) to a world-space position, where (0,0) is the map's top-left corner. */
        [[nodiscard]] Point TileToWorldPosition(int x, int y) const;

        /** @brief Converts a world-space position to tile coordinates. */
        [[nodiscard]] Point WorldToTilePosition(const Vector2& worldPosition) const;

        /**
         * @brief Transfers ownership of @p texture to this tilemap, keeping it alive for as long as the tilemap is.
         * Used internally by TilemapFactory::Build to give loaded textures a stable, non-owning-pointer-safe home;
         * not intended for general use. See this header's own comment for the full rationale.
         * Declared (not defined inline): an inline body would be instantiated -- and would
         * therefore need `Texture2D` complete -- in every TU that includes this header, not just
         * ones that actually call it, since ordinary (non-template) inline member functions are
         * compiled as part of the enclosing class definition wherever it's parsed.
         */
        void AddOwnedTexture(std::unique_ptr<Texture2D> texture);

    private:
        [[nodiscard]] Rectangle CalculateIsometricBounds() const;
        [[nodiscard]] Point TileToWorldOrthogonal(int x, int y) const;
        [[nodiscard]] Point TileToWorldIsometric(int x, int y) const;
        [[nodiscard]] Point TileToWorldStaggered(int x, int y) const;
        [[nodiscard]] Point TileToWorldHexagonal(int x, int y) const;
        [[nodiscard]] Point WorldToTileOrthogonal(const Vector2& worldPosition) const;
        [[nodiscard]] Point WorldToTileIsometric(const Vector2& worldPosition) const;
        [[nodiscard]] Point WorldToTileStaggered(const Vector2& worldPosition) const;
        [[nodiscard]] Point WorldToTileHexagonal(const Vector2& worldPosition) const;

        std::string name_;
        int width_;
        int height_;
        int tileWidth_;
        int tileHeight_;
        TilemapOrientation orientation_;
        int hexSideLength_ = 0;
        TilemapStaggerAxis staggerAxis_ = TilemapStaggerAxis::X;
        TilemapStaggerIndex staggerIndex_ = TilemapStaggerIndex::Even;
        std::optional<Color> backgroundColor_;
        Vector2 parallaxOrigin_;
        Vector2 worldPosition_;
        int worldDepth_ = 0;
        TilemapProperties properties_;
        TilemapTilesetCollection tilesets_;
        TilemapLayerCollection layers_;
        std::vector<std::unique_ptr<Texture2D>> ownedTextures_;
    };
}
