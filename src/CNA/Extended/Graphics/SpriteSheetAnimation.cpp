// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/SpriteSheetAnimation.hpp"

#include <utility>

namespace CNA::Extended::Graphics
{
    SpriteSheetAnimation::SpriteSheetAnimation(const std::string& name,
        std::vector<std::shared_ptr<SpriteSheetAnimationFrame>> frames, bool isLooping, bool isReversed, bool isPingPong)
        : name_(name), frames_(std::move(frames)), isLooping_(isLooping), isReversed_(isReversed), isPingPong_(isPingPong)
    {
        frameInterfaces_.reserve(frames_.size());
        for (const std::shared_ptr<SpriteSheetAnimationFrame>& frame : frames_)
        {
            frameInterfaces_.push_back(frame.get());
        }
    }
}
