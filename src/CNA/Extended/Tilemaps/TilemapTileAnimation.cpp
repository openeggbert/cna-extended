// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapTileAnimation.hpp"

#include "System/InvalidOperationException.hpp"

#include <cmath>
#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapTileAnimation::TilemapTileAnimation(std::vector<TilemapTileAnimationFrame> frames) : frames_(std::move(frames)), totalDuration_(0.0f)
    {
        for (const TilemapTileAnimationFrame& frame : frames_)
        {
            totalDuration_ += frame.Duration;
        }
    }

    void TilemapTileAnimation::Update(float deltaTime)
    {
        if (frames_.empty())
        {
            return;
        }

        elapsedTime_ += deltaTime;

        while (elapsedTime_ >= frames_[static_cast<std::size_t>(currentFrameIndex_)].Duration)
        {
            elapsedTime_ -= frames_[static_cast<std::size_t>(currentFrameIndex_)].Duration;
            currentFrameIndex_ = (currentFrameIndex_ + 1) % static_cast<int>(frames_.size());
        }
    }

    const TilemapTileAnimationFrame& TilemapTileAnimation::GetFrameAtTime(float time) const
    {
        if (frames_.empty())
        {
            throw System::InvalidOperationException("Animation has no frames");
        }

        const float totalDuration = totalDuration_;
        if (totalDuration <= 0.0f)
        {
            return frames_[0];
        }

        time = std::fmod(time, totalDuration);
        if (time < 0.0f)
        {
            time += totalDuration;
        }

        float accumulated = 0.0f;
        for (const TilemapTileAnimationFrame& frame : frames_)
        {
            accumulated += frame.Duration;
            if (time < accumulated)
            {
                return frame;
            }
        }

        // Should never reach here, but return last frame as fallback
        return frames_[frames_.size() - 1];
    }

    void TilemapTileAnimation::Reset()
    {
        currentFrameIndex_ = 0;
        elapsedTime_ = 0.0f;
    }
}
