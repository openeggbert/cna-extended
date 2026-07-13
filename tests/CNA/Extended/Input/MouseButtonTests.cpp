// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below.
#include "CNA/Extended/Input/MouseButton.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input
{
    TEST(MouseButtonTests, OrCombinesFlags)
    {
        const MouseButton combined = MouseButton::Left | MouseButton::Right;
        EXPECT_EQ(combined & MouseButton::Left, MouseButton::Left);
        EXPECT_EQ(combined & MouseButton::Right, MouseButton::Right);
        EXPECT_EQ(combined & MouseButton::Middle, MouseButton::None);
    }

    TEST(MouseButtonTests, OrAssignAccumulatesFlags)
    {
        MouseButton buttons = MouseButton::None;
        buttons |= MouseButton::XButton1;
        buttons |= MouseButton::XButton2;

        EXPECT_EQ(buttons & MouseButton::XButton1, MouseButton::XButton1);
        EXPECT_EQ(buttons & MouseButton::XButton2, MouseButton::XButton2);
    }
}
