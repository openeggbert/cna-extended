// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/KeyboardStateExtended.hpp"

namespace CNA::Extended::Input
{
    KeyboardStateExtended::KeyboardStateExtended(const KeyboardState& currentKeyboardState, const KeyboardState& previousKeyboardState)
        : currentKeyboardState_(currentKeyboardState), previousKeyboardState_(previousKeyboardState)
    {
    }

    bool KeyboardStateExtended::getCapsLockProperty() const
    {
        return currentKeyboardState_.IsKeyDown(Keys::CapsLock);
    }

    bool KeyboardStateExtended::getNumLockProperty() const
    {
        return currentKeyboardState_.IsKeyDown(Keys::NumLock);
    }

    bool KeyboardStateExtended::IsShiftDown() const
    {
        return currentKeyboardState_.IsKeyDown(Keys::LeftShift) || currentKeyboardState_.IsKeyDown(Keys::RightShift);
    }

    bool KeyboardStateExtended::IsControlDown() const
    {
        return currentKeyboardState_.IsKeyDown(Keys::LeftControl) || currentKeyboardState_.IsKeyDown(Keys::RightControl);
    }

    bool KeyboardStateExtended::IsAltDown() const
    {
        return currentKeyboardState_.IsKeyDown(Keys::LeftAlt) || currentKeyboardState_.IsKeyDown(Keys::RightAlt);
    }

    bool KeyboardStateExtended::IsKeyDown(Keys key) const
    {
        return currentKeyboardState_.IsKeyDown(key);
    }

    bool KeyboardStateExtended::IsKeyUp(Keys key) const
    {
        return currentKeyboardState_.IsKeyUp(key);
    }

    int KeyboardStateExtended::getPressedKeyCountProperty() const
    {
        return static_cast<int>(currentKeyboardState_.GetPressedKeys().size());
    }

    std::vector<Keys> KeyboardStateExtended::GetPressedKeys() const
    {
        return currentKeyboardState_.GetPressedKeys();
    }

    bool KeyboardStateExtended::WasKeyReleased(Keys key) const
    {
        return previousKeyboardState_.IsKeyDown(key) && currentKeyboardState_.IsKeyUp(key);
    }

    bool KeyboardStateExtended::WasKeyPressed(Keys key) const
    {
        return previousKeyboardState_.IsKeyUp(key) && currentKeyboardState_.IsKeyDown(key);
    }

    bool KeyboardStateExtended::WasAnyKeyJustDown() const
    {
        return !previousKeyboardState_.GetPressedKeys().empty();
    }
}
