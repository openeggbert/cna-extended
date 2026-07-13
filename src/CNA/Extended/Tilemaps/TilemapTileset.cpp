// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/InvalidOperationException.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapTileset::TilemapTileset(
        std::string name, Texture2D* texture, int tileWidth, int tileHeight, int tileCount, int columns, int spacing, int margin)
        : name_(std::move(name)),
          texture_(texture),
          tileWidth_(tileWidth),
          tileHeight_(tileHeight),
          tileCount_(tileCount),
          columns_(columns),
          spacing_(spacing),
          margin_(margin),
          maxLocalId_(columns > 0 ? tileCount - 1 : -1)
    {
    }

    void TilemapTileset::AddTileData(std::unique_ptr<TilemapTileData> tileData)
    {
        TilemapTileData* raw = tileData.get();
        const int localId = raw->getLocalIdProperty();

        tileData_[localId] = std::move(tileData);

        if (raw->getAnimationProperty() != nullptr && !raw->getAnimationProperty()->getFramesProperty().empty())
        {
            animatedTileData_.push_back(raw);
        }

        if (localId > maxLocalId_)
        {
            maxLocalId_ = localId;
        }
    }

    TilemapTileData* TilemapTileset::GetTileData(int localId) const
    {
        const auto it = tileData_.find(localId);
        if (it != tileData_.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    bool TilemapTileset::ContainsGlobalId(int globalTileId) const
    {
        const int localId = globalTileId - firstGlobalId_;
        return localId >= 0 && localId <= maxLocalId_;
    }

    void TilemapTileset::GetRenderSource(int localId, Texture2D*& texture, Rectangle& sourceRect) const
    {
        TilemapTileData* data = GetTileData(localId);
        int renderLocalId = localId;

        if (data != nullptr && data->getAnimationProperty() != nullptr)
        {
            renderLocalId = data->getAnimationProperty()->getCurrentFrameProperty().TileId;
            data = GetTileData(renderLocalId);
        }

        if (data != nullptr && data->getCustomImageProperty() != nullptr)
        {
            texture = data->getCustomImageProperty();
            sourceRect = Rectangle(0, 0, texture->getWidthProperty(), texture->getHeightProperty());
            return;
        }

        texture = texture_;
        sourceRect = GetTileRegion(renderLocalId);
    }

    Rectangle TilemapTileset::GetTileRegion(int localId) const
    {
        if (columns_ == 0)
        {
            throw System::InvalidOperationException("GetTileRegion is not valid for image collection tilesets. Use GetRenderSource instead.");
        }

        const int column = localId % columns_;
        const int row = localId / columns_;

        const int x = margin_ + column * (tileWidth_ + spacing_);
        const int y = margin_ + row * (tileHeight_ + spacing_);

        return Rectangle(x, y, tileWidth_, tileHeight_);
    }
}
