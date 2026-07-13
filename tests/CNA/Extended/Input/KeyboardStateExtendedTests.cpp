// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below.
#include "CNA/Extended/Input/KeyboardStateExtended.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input
{
    TEST(KeyboardStateExtendedTests, IsKeyDownReflectsCurrentState)
    {
        const KeyboardState current{Keys::A};
        const KeyboardState previous;
        const KeyboardStateExtended state(current, previous);

        EXPECT_TRUE(state.IsKeyDown(Keys::A));
        EXPECT_FALSE(state.IsKeyDown(Keys::B));
        EXPECT_TRUE(state.IsKeyUp(Keys::B));
    }

    TEST(KeyboardStateExtendedTests, WasKeyPressedDetectsUpToDownTransition)
    {
        const KeyboardState previous;
        const KeyboardState current{Keys::Space};
        const KeyboardStateExtended state(current, previous);

        EXPECT_TRUE(state.WasKeyPressed(Keys::Space));
        EXPECT_FALSE(state.WasKeyReleased(Keys::Space));
    }

    TEST(KeyboardStateExtendedTests, WasKeyReleasedDetectsDownToUpTransition)
    {
        const KeyboardState previous{Keys::Space};
        const KeyboardState current;
        const KeyboardStateExtended state(current, previous);

        EXPECT_TRUE(state.WasKeyReleased(Keys::Space));
        EXPECT_FALSE(state.WasKeyPressed(Keys::Space));
    }

    TEST(KeyboardStateExtendedTests, ShiftControlAltDownCheckEitherSide)
    {
        const KeyboardState current{Keys::RightShift, Keys::LeftControl};
        const KeyboardStateExtended state(current, KeyboardState());

        EXPECT_TRUE(state.IsShiftDown());
        EXPECT_TRUE(state.IsControlDown());
        EXPECT_FALSE(state.IsAltDown());
    }

    TEST(KeyboardStateExtendedTests, CapsLockAndNumLockUseIsKeyDown)
    {
        const KeyboardState current{Keys::CapsLock, Keys::NumLock};
        const KeyboardStateExtended state(current, KeyboardState());

        EXPECT_TRUE(state.getCapsLockProperty());
        EXPECT_TRUE(state.getNumLockProperty());
    }

    TEST(KeyboardStateExtendedTests, PressedKeyCountAndGetPressedKeysMatchCurrentState)
    {
        const KeyboardState current{Keys::A, Keys::B, Keys::C};
        const KeyboardStateExtended state(current, KeyboardState());

        EXPECT_EQ(state.getPressedKeyCountProperty(), 3);
        EXPECT_EQ(state.GetPressedKeys().size(), 3u);
    }

    TEST(KeyboardStateExtendedTests, WasAnyKeyJustDownReflectsPreviousState)
    {
        const KeyboardState previousWithKeys{Keys::A};
        const KeyboardState emptyCurrent;
        const KeyboardStateExtended withPrevious(emptyCurrent, previousWithKeys);
        EXPECT_TRUE(withPrevious.WasAnyKeyJustDown());

        const KeyboardState emptyPrevious;
        const KeyboardStateExtended withoutPrevious(emptyCurrent, emptyPrevious);
        EXPECT_FALSE(withoutPrevious.WasAnyKeyJustDown());
    }
}
