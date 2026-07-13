// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/SpriteSheetAnimationFrame.cs. Upstream's class is
// `internal`; C++ has no assembly-level `internal` visibility, so this is a regular public class,
// matching CollisionShapeKind2D.hpp/ActorPairKey.hpp's precedent elsewhere in this port.
// Header-only: trivial immutable data holder, matching this project's convention for such types.
#pragma once

#include "CNA/Extended/Animations/IAnimationFrame.hpp"
#include "System/TimeSpan.hpp"

namespace CNA::Extended::Graphics
{
    using System::TimeSpan;

    /** @brief A single frame within a sprite sheet animation: a region index and display duration. */
    class SpriteSheetAnimationFrame final : public Animations::IAnimationFrame
    {
    public:
        SpriteSheetAnimationFrame(int index, const TimeSpan& duration) : frameIndex_(index), duration_(duration) {}

        /** @brief Gets the index of the frame in the overall sprite sheet's texture atlas. */
        [[nodiscard]] int getFrameIndexProperty() const override { return frameIndex_; }

        /** @brief Gets the total duration this frame should be displayed during an animation. */
        [[nodiscard]] TimeSpan getDurationProperty() const override { return duration_; }

    private:
        int frameIndex_;
        TimeSpan duration_;
    };
}
