// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/SpriteSheet.hpp"

#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"

namespace CNA::Extended::Graphics
{
    SpriteSheet::SpriteSheet(const std::string& name, Texture2DAtlas& textureAtlas) : textureAtlas_(&textureAtlas), name_(name)
    {
    }

    int SpriteSheet::getAnimationCountProperty() const
    {
        return animations_.getCountProperty();
    }

    Sprite SpriteSheet::CreateSprite(int regionIndex) const
    {
        return textureAtlas_->CreateSprite(regionIndex);
    }

    Sprite SpriteSheet::CreateSprite(const std::string& regionName) const
    {
        return textureAtlas_->CreateSprite(regionName);
    }

    void SpriteSheet::DefineAnimation(const std::string& name, const std::function<void(SpriteSheetAnimationBuilder&)>& buildAction)
    {
        SpriteSheetAnimationBuilder builder(name, *this);
        buildAction(builder);
        std::shared_ptr<SpriteSheetAnimation> definition = builder.Build();
        animations_.Add(name, definition);
    }
}
