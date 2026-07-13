// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/IAnimation.cs. Upstream's `ReadOnlySpan<IAnimationFrame>
// Frames { get; }` has no direct C++ equivalent (interface properties cannot return a `std::span` over
// storage the implementation owns without also exposing that storage's lifetime); translated as
// `const std::vector<IAnimationFrame*>&`, matching this project's general non-owning-pointer-element
// convention for interface-exposed collections of other interface types elsewhere in this port.
#pragma once

#include "CNA/Extended/Animations/IAnimationFrame.hpp"

#include <string>
#include <vector>

namespace CNA::Extended::Animations
{
    /** @brief Defines a named sequence of animation frames with looping/direction behavior. */
    class IAnimation
    {
    public:
        virtual ~IAnimation() = default;

        /** @brief Gets the name of this animation. */
        [[nodiscard]] virtual const std::string& getNameProperty() const = 0;

        /** @brief Gets the frames that make up this animation, in playback order. */
        [[nodiscard]] virtual const std::vector<IAnimationFrame*>& getFramesProperty() const = 0;

        /** @brief Gets the number of frames in this animation. */
        [[nodiscard]] virtual int getFrameCountProperty() const = 0;

        /** @brief Gets whether this animation loops when it reaches its last frame. */
        [[nodiscard]] virtual bool getIsLoopingProperty() const = 0;

        /** @brief Gets whether this animation plays back from the last frame to the first. */
        [[nodiscard]] virtual bool getIsReversedProperty() const = 0;

        /** @brief Gets whether this animation reverses direction at each end instead of looping back to the start. */
        [[nodiscard]] virtual bool getIsPingPongProperty() const = 0;
    };
}
