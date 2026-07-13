// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Tilemap.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    Tilemap::Tilemap(std::string name, int width, int height, int tileWidth, int tileHeight, TilemapOrientation orientation)
        : name_(std::move(name)), width_(width), height_(height), tileWidth_(tileWidth), tileHeight_(tileHeight), orientation_(orientation)
    {
    }

    Tilemap::~Tilemap() = default;

    void Tilemap::AddOwnedTexture(std::unique_ptr<Texture2D> texture)
    {
        ownedTextures_.push_back(std::move(texture));
    }

    Rectangle Tilemap::getWorldBoundsProperty() const
    {
        switch (orientation_)
        {
            case TilemapOrientation::Isometric:
                return CalculateIsometricBounds();
            case TilemapOrientation::Orthogonal:
            case TilemapOrientation::Staggered:
            case TilemapOrientation::Hexagonal:
            default:
                return Rectangle(0, 0, width_ * tileWidth_, height_ * tileHeight_);
        }
    }

    Point Tilemap::TileToWorldPosition(int x, int y) const
    {
        switch (orientation_)
        {
            case TilemapOrientation::Isometric:
                return TileToWorldIsometric(x, y);
            case TilemapOrientation::Staggered:
                return TileToWorldStaggered(x, y);
            case TilemapOrientation::Hexagonal:
                return TileToWorldHexagonal(x, y);
            case TilemapOrientation::Orthogonal:
            default:
                return TileToWorldOrthogonal(x, y);
        }
    }

    Point Tilemap::WorldToTilePosition(const Vector2& worldPosition) const
    {
        switch (orientation_)
        {
            case TilemapOrientation::Isometric:
                return WorldToTileIsometric(worldPosition);
            case TilemapOrientation::Staggered:
                return WorldToTileStaggered(worldPosition);
            case TilemapOrientation::Hexagonal:
                return WorldToTileHexagonal(worldPosition);
            case TilemapOrientation::Orthogonal:
            default:
                return WorldToTileOrthogonal(worldPosition);
        }
    }

    Rectangle Tilemap::CalculateIsometricBounds() const
    {
        const int worldWidth = (width_ + height_) * (tileWidth_ / 2);
        const int worldHeight = (width_ + height_) * (tileHeight_ / 2);

        return Rectangle(0, 0, worldWidth, worldHeight);
    }

    Point Tilemap::TileToWorldOrthogonal(int x, int y) const
    {
        return Point(x * tileWidth_, y * tileHeight_);
    }

    Point Tilemap::TileToWorldIsometric(int x, int y) const
    {
        const int worldX = (x - y) * (tileWidth_ / 2);
        const int worldY = (x + y) * (tileHeight_ / 2);

        return Point(worldX, worldY);
    }

    Point Tilemap::TileToWorldStaggered(int x, int y) const
    {
        if (staggerAxis_ == TilemapStaggerAxis::Y)
        {
            const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (y % 2 != 0) : (y % 2 == 0);
            const int worldX = x * tileWidth_ + (isStaggered ? tileWidth_ / 2 : 0);
            const int worldY = y * (tileHeight_ / 2);
            return Point(worldX, worldY);
        }

        const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (x % 2 != 0) : (x % 2 == 0);
        const int worldX = x * (tileWidth_ / 2);
        const int worldY = y * tileHeight_ + (isStaggered ? tileHeight_ / 2 : 0);
        return Point(worldX, worldY);
    }

    Point Tilemap::TileToWorldHexagonal(int x, int y) const
    {
        if (staggerAxis_ == TilemapStaggerAxis::Y)
        {
            // Rows are staggered. Vertical step = (tileHeight + hexSideLength) / 2.
            const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (y % 2 != 0) : (y % 2 == 0);
            const int worldX = x * tileWidth_ + (isStaggered ? tileWidth_ / 2 : 0);
            const int worldY = y * (tileHeight_ + hexSideLength_) / 2;
            return Point(worldX, worldY);
        }

        // Columns are staggered. Horizontal step = (tileWidth + hexSideLength) / 2.
        const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (x % 2 != 0) : (x % 2 == 0);
        const int worldX = x * (tileWidth_ + hexSideLength_) / 2;
        const int worldY = y * tileHeight_ + (isStaggered ? tileHeight_ / 2 : 0);
        return Point(worldX, worldY);
    }

    Point Tilemap::WorldToTileOrthogonal(const Vector2& worldPosition) const
    {
        const int tileX = static_cast<int>(worldPosition.X / static_cast<float>(tileWidth_));
        const int tileY = static_cast<int>(worldPosition.Y / static_cast<float>(tileHeight_));

        return Point(tileX, tileY);
    }

    Point Tilemap::WorldToTileIsometric(const Vector2& worldPosition) const
    {
        const float halfTileWidth = static_cast<float>(tileWidth_) * 0.5f;
        const float halfTileHeight = static_cast<float>(tileHeight_) * 0.5f;

        const float normalizedX = worldPosition.X / halfTileWidth;
        const float normalizedY = worldPosition.Y / halfTileHeight;

        const int tileX = static_cast<int>((normalizedX + normalizedY) * 0.5f);
        const int tileY = static_cast<int>((normalizedY - normalizedX) * 0.5f);

        return Point(tileX, tileY);
    }

    Point Tilemap::WorldToTileStaggered(const Vector2& worldPosition) const
    {
        if (staggerAxis_ == TilemapStaggerAxis::Y)
        {
            const int rowStep = tileHeight_ / 2;
            const int row = static_cast<int>(worldPosition.Y / static_cast<float>(rowStep));
            const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (row % 2 != 0) : (row % 2 == 0);
            const int col = static_cast<int>((worldPosition.X - (isStaggered ? static_cast<float>(tileWidth_) / 2.0f : 0.0f)) / static_cast<float>(tileWidth_));
            return Point(col, row);
        }

        const int colStep = tileWidth_ / 2;
        const int col = static_cast<int>(worldPosition.X / static_cast<float>(colStep));
        const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (col % 2 != 0) : (col % 2 == 0);
        const int row = static_cast<int>((worldPosition.Y - (isStaggered ? static_cast<float>(tileHeight_) / 2.0f : 0.0f)) / static_cast<float>(tileHeight_));
        return Point(col, row);
    }

    Point Tilemap::WorldToTileHexagonal(const Vector2& worldPosition) const
    {
        if (staggerAxis_ == TilemapStaggerAxis::Y)
        {
            const int rowStep = (tileHeight_ + hexSideLength_) / 2;
            const int row = static_cast<int>(worldPosition.Y / static_cast<float>(rowStep));
            const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (row % 2 != 0) : (row % 2 == 0);
            const int col = static_cast<int>((worldPosition.X - (isStaggered ? static_cast<float>(tileWidth_) / 2.0f : 0.0f)) / static_cast<float>(tileWidth_));
            return Point(col, row);
        }

        const int colStep = (tileWidth_ + hexSideLength_) / 2;
        const int col = static_cast<int>(worldPosition.X / static_cast<float>(colStep));
        const bool isStaggered = staggerIndex_ == TilemapStaggerIndex::Odd ? (col % 2 != 0) : (col % 2 == 0);
        const int row = static_cast<int>((worldPosition.Y - (isStaggered ? static_cast<float>(tileHeight_) / 2.0f : 0.0f)) / static_cast<float>(tileHeight_));
        return Point(col, row);
    }
}
