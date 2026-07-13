// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapTileAnimation.cs. Upstream's
// `TilemapTileAnimationFrame[] Frames` -> `std::vector<TilemapTileAnimationFrame>`, matching this
// project's established C# array -> std::vector convention.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapTileAnimationFrame.hpp"

#include <vector>

namespace CNA::Extended::Tilemaps
{
    /** @brief A frame-based animation for a tile: a sequence of TilemapTileAnimationFrame entries played in a loop. */
    class TilemapTileAnimation
    {
    public:
        /** @brief Creates an animation from @p frames, precomputing the total duration. */
        explicit TilemapTileAnimation(std::vector<TilemapTileAnimationFrame> frames);

        /** @brief Gets the animation frames. */
        [[nodiscard]] const std::vector<TilemapTileAnimationFrame>& getFramesProperty() const { return frames_; }

        /** @brief Gets the total duration of the animation in seconds (sum of all frame durations). */
        [[nodiscard]] float getTotalDurationProperty() const { return totalDuration_; }

        /** @brief Gets the current frame index. */
        [[nodiscard]] int getCurrentFrameIndexProperty() const { return currentFrameIndex_; }

        /** @brief Gets the current frame. */
        [[nodiscard]] const TilemapTileAnimationFrame& getCurrentFrameProperty() const { return frames_[static_cast<std::size_t>(currentFrameIndex_)]; }

        /** @brief Advances playback by @p deltaTime seconds, wrapping to the next frame(s) as their durations elapse. No-op if there are no frames. */
        void Update(float deltaTime);

        /**
         * @brief Returns the animation frame that would be active at playback position @p time.
         * @param time Playback position in seconds; values outside [0, TotalDuration) are wrapped.
         * @throws System::InvalidOperationException the animation has no frames.
         */
        [[nodiscard]] const TilemapTileAnimationFrame& GetFrameAtTime(float time) const;

        /** @brief Resets playback to the first frame with zero elapsed time. */
        void Reset();

    private:
        std::vector<TilemapTileAnimationFrame> frames_;
        float totalDuration_;
        int currentFrameIndex_ = 0;
        float elapsedTime_ = 0.0f;
    };
}
