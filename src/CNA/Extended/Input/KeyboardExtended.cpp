// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/KeyboardExtended.hpp"

#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

namespace CNA::Extended::Input
{
    using Microsoft::Xna::Framework::Input::Keyboard;

    KeyboardState KeyboardExtended::currentKeyboardState_;
    KeyboardState KeyboardExtended::previousKeyboardState_;

    KeyboardStateExtended KeyboardExtended::GetState()
    {
        return KeyboardStateExtended(currentKeyboardState_, previousKeyboardState_);
    }

    void KeyboardExtended::Update()
    {
        previousKeyboardState_ = currentKeyboardState_;
        currentKeyboardState_ = Keyboard::GetState();
    }
}
