// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below.
#include "CNA/Extended/Input/InputListeners/KeyboardEventArgs.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input::InputListeners
{
    TEST(KeyboardEventArgsTests, DerivesNoModifiersFromPlainState)
    {
        const KeyboardState state{Keys::A};
        const KeyboardEventArgs args(Keys::A, state);

        EXPECT_EQ(args.getKeyProperty(), Keys::A);
        EXPECT_EQ(args.getModifiersProperty(), KeyboardModifiers::None);
    }

    TEST(KeyboardEventArgsTests, DerivesControlShiftAltModifiersFromEitherSide)
    {
        const KeyboardState state{Keys::A, Keys::RightControl, Keys::LeftShift, Keys::RightAlt};
        const KeyboardEventArgs args(Keys::A, state);

        const KeyboardModifiers modifiers = args.getModifiersProperty();
        EXPECT_EQ(modifiers & KeyboardModifiers::Control, KeyboardModifiers::Control);
        EXPECT_EQ(modifiers & KeyboardModifiers::Shift, KeyboardModifiers::Shift);
        EXPECT_EQ(modifiers & KeyboardModifiers::Alt, KeyboardModifiers::Alt);
    }

    TEST(KeyboardEventArgsTests, CharacterReturnsLowercaseWithoutShift)
    {
        const KeyboardEventArgs args(Keys::A, KeyboardState{Keys::A});
        ASSERT_TRUE(args.getCharacterProperty().has_value());
        EXPECT_EQ(args.getCharacterProperty().value(), 'a');
    }

    TEST(KeyboardEventArgsTests, CharacterReturnsUppercaseWithShift)
    {
        const KeyboardEventArgs args(Keys::A, KeyboardState{Keys::A, Keys::LeftShift});
        ASSERT_TRUE(args.getCharacterProperty().has_value());
        EXPECT_EQ(args.getCharacterProperty().value(), 'A');
    }

    TEST(KeyboardEventArgsTests, CharacterReturnsDigitWithoutShiftAndSymbolWithShift)
    {
        const KeyboardEventArgs plain(Keys::D1, KeyboardState{Keys::D1});
        ASSERT_TRUE(plain.getCharacterProperty().has_value());
        EXPECT_EQ(plain.getCharacterProperty().value(), '1');

        const KeyboardEventArgs shifted(Keys::D1, KeyboardState{Keys::D1, Keys::LeftShift});
        ASSERT_TRUE(shifted.getCharacterProperty().has_value());
        EXPECT_EQ(shifted.getCharacterProperty().value(), '!');
    }

    TEST(KeyboardEventArgsTests, CharacterIsNulloptForKeysWithoutAPrintableRepresentation)
    {
        const KeyboardEventArgs args(Keys::F1, KeyboardState{Keys::F1});
        EXPECT_FALSE(args.getCharacterProperty().has_value());
    }
}
