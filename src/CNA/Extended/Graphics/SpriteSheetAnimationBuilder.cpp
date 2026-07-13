// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp"

#include "CNA/Extended/Graphics/SpriteSheet.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"

namespace CNA::Extended::Graphics
{
    SpriteSheetAnimationBuilder& SpriteSheetAnimationBuilder::AddFrame(int regionIndex, const TimeSpan& duration)
    {
        frames_.push_back(std::make_shared<SpriteSheetAnimationFrame>(regionIndex, duration));
        return *this;
    }

    SpriteSheetAnimationBuilder& SpriteSheetAnimationBuilder::AddFrame(const std::string& regionName, const TimeSpan& duration)
    {
        const int index = spriteSheet_->getTextureAtlasProperty().GetIndexOfRegion(regionName);
        return AddFrame(index, duration);
    }

    std::shared_ptr<SpriteSheetAnimation> SpriteSheetAnimationBuilder::Build() const
    {
        return std::make_shared<SpriteSheetAnimation>(name_, frames_, isLooping_, isReversed_, isPingPong_);
    }
}
