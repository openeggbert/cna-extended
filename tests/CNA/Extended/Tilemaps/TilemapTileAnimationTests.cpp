// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Tilemaps/TilemapTileAnimationTests.cs.
#include "CNA/Extended/Tilemaps/TilemapTileAnimation.hpp"

#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    TEST(TilemapTileAnimationTests, TotalDurationSumsAllFrameDurations)
    {
        std::vector<TilemapTileAnimationFrame> frames{
            TilemapTileAnimationFrame(0, 0.1f), TilemapTileAnimationFrame(1, 0.2f), TilemapTileAnimationFrame(2, 0.3f)};
        const TilemapTileAnimation animation(std::move(frames));

        EXPECT_NEAR(animation.getTotalDurationProperty(), 0.6f, 0.001f);
    }

    TEST(TilemapTileAnimationTests, UpdateWithSmallDeltaStaysOnSameFrame)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(0, 0.5f), TilemapTileAnimationFrame(1, 0.5f)};
        TilemapTileAnimation animation(std::move(frames));

        animation.Update(0.1f);

        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 0);
        EXPECT_EQ(animation.getCurrentFrameProperty().TileId, 0);
    }

    TEST(TilemapTileAnimationTests, UpdateWithExactFrameDurationAdvancesToNextFrame)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(0, 0.5f), TilemapTileAnimationFrame(1, 0.5f)};
        TilemapTileAnimation animation(std::move(frames));

        animation.Update(0.5f);

        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 1);
        EXPECT_EQ(animation.getCurrentFrameProperty().TileId, 1);
    }

    TEST(TilemapTileAnimationTests, UpdateWithLargeDeltaAdvancesMultipleFrames)
    {
        std::vector<TilemapTileAnimationFrame> frames{
            TilemapTileAnimationFrame(0, 0.1f), TilemapTileAnimationFrame(1, 0.1f), TilemapTileAnimationFrame(2, 0.1f)};
        TilemapTileAnimation animation(std::move(frames));

        // 0.25s: crosses frame 0 (0.1s) and frame 1 (0.1s), lands in frame 2
        animation.Update(0.25f);

        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 2);
        EXPECT_EQ(animation.getCurrentFrameProperty().TileId, 2);
    }

    TEST(TilemapTileAnimationTests, UpdateWhenReachingEndWrapsToBeginning)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(0, 0.1f), TilemapTileAnimationFrame(1, 0.1f)};
        TilemapTileAnimation animation(std::move(frames));

        // 0.25s exceeds total duration of 0.2s, wrapping 0.05s into the first frame
        animation.Update(0.25f);

        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 0);
        EXPECT_EQ(animation.getCurrentFrameProperty().TileId, 0);
    }

    TEST(TilemapTileAnimationTests, ResetResetsToFirstFrame)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(0, 0.1f), TilemapTileAnimationFrame(1, 0.1f)};
        TilemapTileAnimation animation(std::move(frames));
        animation.Update(0.15f);

        animation.Reset();

        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 0);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeWithinFirstFrameReturnsFirstFrame)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(10, 0.1f), TilemapTileAnimationFrame(11, 0.2f)};
        const TilemapTileAnimation animation(std::move(frames));

        EXPECT_EQ(animation.GetFrameAtTime(0.05f).TileId, 10);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeInSecondFrameReturnsSecondFrame)
    {
        std::vector<TilemapTileAnimationFrame> frames{
            TilemapTileAnimationFrame(10, 0.1f), TilemapTileAnimationFrame(11, 0.2f), TilemapTileAnimationFrame(12, 0.3f)};
        const TilemapTileAnimation animation(std::move(frames));

        EXPECT_EQ(animation.GetFrameAtTime(0.15f).TileId, 11);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeBeyondTotalDurationWraps)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(10, 0.1f), TilemapTileAnimationFrame(11, 0.2f)};
        const TilemapTileAnimation animation(std::move(frames));

        // 0.35s exceeds total duration of 0.3s, wrapping 0.05s into the first frame
        EXPECT_EQ(animation.GetFrameAtTime(0.35f).TileId, 10);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeWithNegativeTimeWrapsFromEnd)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(10, 0.1f), TilemapTileAnimationFrame(11, 0.2f)};
        const TilemapTileAnimation animation(std::move(frames));

        EXPECT_EQ(animation.GetFrameAtTime(-0.1f).TileId, 11);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeWithEmptyFramesThrowsInvalidOperationException)
    {
        const TilemapTileAnimation animation({});

        EXPECT_THROW((void)animation.GetFrameAtTime(0.0f), System::InvalidOperationException);
    }

    TEST(TilemapTileAnimationTests, GetFrameAtTimeDoesNotAffectCurrentState)
    {
        std::vector<TilemapTileAnimationFrame> frames{TilemapTileAnimationFrame(10, 0.1f), TilemapTileAnimationFrame(11, 0.2f)};
        const TilemapTileAnimation animation(std::move(frames));

        const TilemapTileAnimationFrame frame1 = animation.GetFrameAtTime(0.15f);
        const TilemapTileAnimationFrame frame2 = animation.GetFrameAtTime(0.25f);

        EXPECT_EQ(frame1.TileId, 11);
        EXPECT_EQ(frame2.TileId, 11);
        EXPECT_EQ(animation.getCurrentFrameIndexProperty(), 0);
        EXPECT_EQ(animation.getCurrentFrameProperty().TileId, 10);
    }
}
