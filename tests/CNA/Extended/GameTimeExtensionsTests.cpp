// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for GameTimeExtensions.cs. Fresh tests below.
#include "CNA/Extended/GameTimeExtensions.hpp"

#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using System::TimeSpan;

    TEST(GameTimeExtensionsTests, GetElapsedSecondsReturnsElapsedGameTimeInSeconds)
    {
        const GameTime gameTime(TimeSpan::FromSeconds(10.0), TimeSpan::FromSeconds(0.5));
        EXPECT_FLOAT_EQ(GetElapsedSeconds(gameTime), 0.5f);
    }

    TEST(GameTimeExtensionsTests, GetElapsedSecondsIsZeroForZeroElapsedTime)
    {
        const GameTime gameTime;
        EXPECT_FLOAT_EQ(GetElapsedSeconds(gameTime), 0.0f);
    }
}
