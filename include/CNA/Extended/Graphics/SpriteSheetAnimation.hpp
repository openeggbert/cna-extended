// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/SpriteSheetAnimation.cs. `ReadOnlySpan<IAnimationFrame>
// Frames` -> `const std::vector<Animations::IAnimationFrame*>&`, matching
// CNA::Extended::Animations::IAnimation's own established translation of this exact property
// shape. The public constructor is `internal` upstream (built only by
// SpriteSheetAnimationBuilder::Build); kept public per this port's internal-visibility convention.
#pragma once

#include "CNA/Extended/Animations/IAnimation.hpp"
#include "CNA/Extended/Graphics/SpriteSheetAnimationFrame.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Graphics
{
    /** @brief A named, ordered sequence of sprite sheet animation frames with looping/direction behavior. */
    class SpriteSheetAnimation final : public Animations::IAnimation
    {
    public:
        SpriteSheetAnimation(const std::string& name, std::vector<std::shared_ptr<SpriteSheetAnimationFrame>> frames,
            bool isLooping, bool isReversed, bool isPingPong);

        [[nodiscard]] const std::string& getNameProperty() const override { return name_; }
        [[nodiscard]] const std::vector<Animations::IAnimationFrame*>& getFramesProperty() const override { return frameInterfaces_; }
        [[nodiscard]] int getFrameCountProperty() const override { return static_cast<int>(frames_.size()); }
        [[nodiscard]] bool getIsLoopingProperty() const override { return isLooping_; }
        [[nodiscard]] bool getIsReversedProperty() const override { return isReversed_; }
        [[nodiscard]] bool getIsPingPongProperty() const override { return isPingPong_; }

    private:
        std::string name_;
        std::vector<std::shared_ptr<SpriteSheetAnimationFrame>> frames_;
        std::vector<Animations::IAnimationFrame*> frameInterfaces_;
        bool isLooping_;
        bool isReversed_;
        bool isPingPong_;
    };
}
