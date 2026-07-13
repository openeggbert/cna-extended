// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below.
#include "CNA/Extended/Input/InputListeners/GamePadEventArgs.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::PlayerIndex;

    TEST(GamePadEventArgsTests, DefaultsThumbStickStateToZeroWhenNotProvided)
    {
        const GamePadState previous;
        const GamePadState current;
        const GamePadEventArgs args(previous, current, System::TimeSpan::Zero, PlayerIndex::One);
        EXPECT_EQ(args.getThumbStickStateProperty(), Vector2::Zero);
        EXPECT_EQ(args.getTriggerStateProperty(), 0.0f);
    }

    TEST(GamePadEventArgsTests, StoresButtonWhenProvided)
    {
        const GamePadState previous;
        const GamePadState current;
        const GamePadEventArgs args(previous, current, System::TimeSpan::Zero, PlayerIndex::Two, Buttons::A);
        EXPECT_EQ(args.getButtonProperty(), Buttons::A);
        EXPECT_EQ(args.getPlayerIndexProperty(), PlayerIndex::Two);
    }

    TEST(GamePadEventArgsTests, StoresTriggerAndThumbStickState)
    {
        const GamePadState previous;
        const GamePadState current;
        const Vector2 stick(0.5f, -0.5f);
        const GamePadEventArgs args(previous, current, System::TimeSpan::Zero, PlayerIndex::One, Buttons::RightTrigger, 0.75f, stick);

        EXPECT_EQ(args.getTriggerStateProperty(), 0.75f);
        EXPECT_EQ(args.getThumbStickStateProperty(), stick);
    }
}
