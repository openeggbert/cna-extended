// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/IAnimationFrame.cs.
#pragma once

#include "System/TimeSpan.hpp"

namespace CNA::Extended::Animations
{
    using System::TimeSpan;

    /** @brief Defines a single frame within an animation. */
    class IAnimationFrame
    {
    public:
        virtual ~IAnimationFrame() = default;

        /** @brief Gets the index of this frame within the source frame collection (e.g. a Texture2DAtlas region index). */
        [[nodiscard]] virtual int getFrameIndexProperty() const = 0;

        /** @brief Gets the length of time this frame is displayed for. */
        [[nodiscard]] virtual TimeSpan getDurationProperty() const = 0;
    };
}
