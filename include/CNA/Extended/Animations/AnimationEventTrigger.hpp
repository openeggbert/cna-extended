// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/AnimationEventTrigger.cs.
#pragma once

namespace CNA::Extended::Animations
{
    /** @brief Identifies the kind of event raised by an IAnimationController during playback. */
    enum class AnimationEventTrigger
    {
        /** @brief Raised when a frame begins being displayed. */
        FrameBegin,

        /** @brief Raised when a frame finishes being displayed. */
        FrameEnd,

        /** @brief Raised when a frame is skipped over entirely (e.g. a very large elapsed-time update). */
        FrameSkipped,

        /** @brief Raised when a looping animation reaches its end and loops back. */
        AnimationLoop,

        /** @brief Raised when a non-looping animation reaches its end and stops. */
        AnimationCompleted,

        /** @brief Raised when the animation is stopped via Stop(). */
        AnimationStopped
    };
}
