// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/SpriteSheetAnimationBuilder.cs. `sealed class` -> C++
// `final`. The constructor and Build() are `internal` upstream (a builder is only meant to be
// constructed by SpriteSheet::DefineAnimation and built by it); kept public per this port's
// internal-visibility convention. Fluent `AddFrame`/`IsLooping`/`IsReversed`/`IsPingPong` methods
// return `*this` by reference rather than the C# `this` value, since the builder is a regular
// (non-reference-type) C++ object here.
#pragma once

#include "CNA/Extended/Graphics/SpriteSheetAnimation.hpp"
#include "System/TimeSpan.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Graphics
{
    using System::TimeSpan;

    class SpriteSheet;

    /** @brief A fluent builder for SpriteSheetAnimation instances. */
    class SpriteSheetAnimationBuilder final
    {
    public:
        SpriteSheetAnimationBuilder(const std::string& name, SpriteSheet& spriteSheet) : name_(name), spriteSheet_(&spriteSheet) {}

        /** @brief Adds a frame to the animation using the region index and duration. */
        SpriteSheetAnimationBuilder& AddFrame(int regionIndex, const TimeSpan& duration);

        /**
         * @brief Adds a frame to the animation using the region name and duration.
         * @throws std::invalid_argument no region named @p regionName exists in the owning sprite sheet's atlas.
         */
        SpriteSheetAnimationBuilder& AddFrame(const std::string& regionName, const TimeSpan& duration);

        /** @brief Sets whether the animation should loop. */
        SpriteSheetAnimationBuilder& IsLooping(bool isLooping)
        {
            isLooping_ = isLooping;
            return *this;
        }

        /** @brief Sets whether the animation should play in reverse. */
        SpriteSheetAnimationBuilder& IsReversed(bool isReversed)
        {
            isReversed_ = isReversed;
            return *this;
        }

        /**
         * @brief Sets whether the animation should ping-pong (reverse direction at the ends)
         * instead of looping back to the start.
         *
         * @note Faithful upstream MonoGame.Extended behavior, not a port defect: ping-pong
         * only takes effect if `IsLooping(true)` is also set. `IsPingPong(true)` alone plays
         * once forward and stops on the last frame instead of bouncing — see
         * `AnimationController::AdvanceFrame`'s `isLooping_`-gated branch.
         */
        SpriteSheetAnimationBuilder& IsPingPong(bool isPingPong)
        {
            isPingPong_ = isPingPong;
            return *this;
        }

        /** @brief Builds the SpriteSheetAnimation from the frames and options accumulated so far. */
        [[nodiscard]] std::shared_ptr<SpriteSheetAnimation> Build() const;

    private:
        std::string name_;
        SpriteSheet* spriteSheet_;
        std::vector<std::shared_ptr<SpriteSheetAnimationFrame>> frames_;
        bool isLooping_ = false;
        bool isReversed_ = false;
        bool isPingPong_ = false;
    };
}
