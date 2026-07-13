// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapImageLayer.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapImageLayer::TilemapImageLayer(std::string name, Texture2D* texture, const Vector2& position)
        : TilemapLayer(std::move(name)), texture_(texture), position_(position)
    {
    }

    Rectangle TilemapImageLayer::getBoundsProperty() const
    {
        if (texture_ == nullptr)
        {
            return Rectangle::Empty;
        }

        return Rectangle(
            static_cast<int>(position_.X), static_cast<int>(position_.Y), texture_->getWidthProperty(), texture_->getHeightProperty());
    }
}
