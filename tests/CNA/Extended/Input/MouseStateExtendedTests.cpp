// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below.
#include "CNA/Extended/Input/MouseStateExtended.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input
{
    namespace
    {
        MouseState MakeState(int x, int y, int scrollWheel, ButtonState leftButton = ButtonState::Released)
        {
            return MouseState(x, y, scrollWheel, leftButton, ButtonState::Released, ButtonState::Released, ButtonState::Released,
                ButtonState::Released);
        }
    }

    TEST(MouseStateExtendedTests, PositionAndDeltaReflectCurrentAndPreviousState)
    {
        const MouseStateExtended state(MakeState(15, 25, 0), MakeState(10, 20, 0));

        EXPECT_EQ(state.getXProperty(), 15);
        EXPECT_EQ(state.getYProperty(), 25);
        EXPECT_EQ(state.getPositionProperty(), Point(15, 25));
        EXPECT_TRUE(state.getPositionChangedProperty());
        EXPECT_EQ(state.getDeltaXProperty(), -5);
        EXPECT_EQ(state.getDeltaYProperty(), -5);
        EXPECT_EQ(state.getDeltaPositionProperty(), Point(-5, -5));
    }

    TEST(MouseStateExtendedTests, PositionUnchangedWhenCoordinatesMatch)
    {
        const MouseStateExtended state(MakeState(10, 20, 0), MakeState(10, 20, 0));
        EXPECT_FALSE(state.getPositionChangedProperty());
    }

    TEST(MouseStateExtendedTests, ScrollWheelDeltaReflectsDifference)
    {
        const MouseStateExtended state(MakeState(0, 0, 120), MakeState(0, 0, 0));
        EXPECT_EQ(state.getScrollWheelValueProperty(), 120);
        EXPECT_EQ(state.getDeltaScrollWheelValueProperty(), -120);
    }

    TEST(MouseStateExtendedTests, IsButtonDownAndUpReflectCurrentState)
    {
        const MouseStateExtended state(MakeState(0, 0, 0, ButtonState::Pressed), MakeState(0, 0, 0, ButtonState::Pressed));

        EXPECT_TRUE(state.IsButtonDown(MouseButton::Left));
        EXPECT_FALSE(state.IsButtonUp(MouseButton::Left));
        EXPECT_TRUE(state.IsButtonUp(MouseButton::Right));
    }

    TEST(MouseStateExtendedTests, WasButtonPressedDetectsUpToDownTransition)
    {
        const MouseStateExtended state(MakeState(0, 0, 0, ButtonState::Pressed), MakeState(0, 0, 0, ButtonState::Released));

        EXPECT_TRUE(state.WasButtonPressed(MouseButton::Left));
        EXPECT_FALSE(state.WasButtonReleased(MouseButton::Left));
    }

    TEST(MouseStateExtendedTests, WasButtonReleasedDetectsDownToUpTransition)
    {
        const MouseStateExtended state(MakeState(0, 0, 0, ButtonState::Released), MakeState(0, 0, 0, ButtonState::Pressed));

        EXPECT_TRUE(state.WasButtonReleased(MouseButton::Left));
        EXPECT_FALSE(state.WasButtonPressed(MouseButton::Left));
    }
}
