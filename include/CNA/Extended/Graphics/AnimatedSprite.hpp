// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/AnimatedSprite.cs. `IAnimationController Controller
// { get; private set; }` and the backing `IAnimation _animation` field are both left unset by the
// single-argument constructor (upstream: `AnimatedSprite(SpriteSheet spriteSheet)` never assigns
// either -- calling CurrentAnimation, Controller-dependent members, or Update before a subsequent
// SetAnimation() call throws NullReferenceException upstream). Translated as nullable
// (std::shared_ptr<SpriteSheetAnimation> / std::unique_ptr<Animations::AnimationController>,
// both null after that constructor), with an explicit std::logic_error thrown by
// getCurrentAnimationProperty()/getControllerProperty()/Update() when accessed before
// SetAnimation() -- preserving the "must call SetAnimation first" contract as a catchable C++
// exception instead of upstream's implicit null-dereference crash.
// `ArgumentNullException.ThrowIfNull(spriteSheet)` is dropped: the constructor takes a reference
// (spriteSheet cannot be null), and upstream's own check is unreachable dead code anyway (the
// `base(spriteSheet.TextureAtlas[0])` initializer above it already dereferences spriteSheet
// first, so a genuinely-null spriteSheet would NPE there, never reaching the check).
#pragma once

#include "CNA/Extended/Animations/AnimationController.hpp"
#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Graphics/SpriteSheet.hpp"
#include "CNA/Extended/Graphics/SpriteSheetAnimation.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/TimeSpan.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::GameTime;
    using System::TimeSpan;

    /** @brief A Sprite that plays a named SpriteSheet animation, advancing its TextureRegion frame-by-frame over time. */
    class AnimatedSprite final : public Sprite
    {
    public:
        /**
         * @brief Initializes an animated sprite showing @p spriteSheet's first region. No
         * animation is playing until SetAnimation() is called (see this file's header comment).
         */
        explicit AnimatedSprite(SpriteSheet& spriteSheet);

        /**
         * @brief Initializes an animated sprite and immediately starts playing @p initialAnimation.
         * @throws System::Collections::Generic::KeyNotFoundException no animation named @p initialAnimation exists.
         */
        AnimatedSprite(SpriteSheet& spriteSheet, const std::string& initialAnimation);

        /**
         * @brief Gets the animation controller used to control the current animation of this sprite.
         * @throws std::logic_error no animation has been set yet (see this file's header comment).
         */
        [[nodiscard]] Animations::IAnimationController& getControllerProperty() const;

        /**
         * @brief Gets the name of the current animation playing.
         * @throws std::logic_error no animation has been set yet (see this file's header comment).
         */
        [[nodiscard]] const std::string& getCurrentAnimationProperty() const;

        /**
         * @brief Sets the animation to use for this animated sprite, starting playback at its first frame.
         * @throws System::Collections::Generic::KeyNotFoundException no animation named @p name exists.
         */
        Animations::IAnimationController& SetAnimation(const std::string& name);

        /**
         * @brief Advances the current animation by gameTime.ElapsedGameTime.
         * @throws std::logic_error no animation has been set yet (see this file's header comment).
         */
        void Update(const GameTime& gameTime);

        /**
         * @brief Advances the current animation by @p elapsedTime.
         * @throws std::logic_error no animation has been set yet (see this file's header comment).
         */
        void Update(const TimeSpan& elapsedTime);

    private:
        SpriteSheet* spriteSheet_;
        std::shared_ptr<SpriteSheetAnimation> animation_;
        std::unique_ptr<Animations::AnimationController> controller_;
    };
}
