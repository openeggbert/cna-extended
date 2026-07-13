// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/KeyboardStateExtended.cs. Upstream has an `#if FNA`
// branch for CapsLock/NumLock (using IsKeyDown(Keys.CapsLock/NumLock) since FNA's KeyboardState
// has no native CapsLock/NumLock properties) and a non-FNA branch (using
// KeyboardState.CapsLock/NumLock directly). CNA mirrors FNA (an established precedent throughout
// this project, e.g. RectangleExtensions), so the FNA branch applies unconditionally here.
//
// `GetPressedKeyCount` (upstream: `_currentKeyboardState.GetPressedKeyCount()`, an FNA extension
// method backed by a `GetPressedKeys().Length` computation -- see KeyboardState.Extensions.cs) ->
// `getPressedKeys().size()`. CNA's own KeyboardState::GetPressedKeys() already returns a
// self-sizing std::vector<Keys>, so there is no separate count-then-fill-array API to port --
// upstream's `GetPressedKeys(Keys[] keys)` fill-into-existing-array overload has no C++
// equivalent need and is intentionally not ported (CNA's GetPressedKeys() already returns a
// complete, appropriately-sized vector in one call).
#pragma once

#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"

#include <vector>

namespace CNA::Extended::Input
{
    using Microsoft::Xna::Framework::Input::KeyboardState;
    using Microsoft::Xna::Framework::Input::Keys;

    /**
     * @brief An extended view of keyboard input state, tracking both the previous and current
     * KeyboardState to expose "was this key just pressed/released" queries.
     */
    struct KeyboardStateExtended
    {
        KeyboardStateExtended() = default;

        /**
         * @brief Initializes a new KeyboardStateExtended value.
         * @param currentKeyboardState The state of keyboard input during the current update cycle.
         * @param previousKeyboardState The state of keyboard input during the previous update cycle.
         */
        KeyboardStateExtended(const KeyboardState& currentKeyboardState, const KeyboardState& previousKeyboardState);

        /** @brief Gets whether the caps lock key is down during the current state. */
        [[nodiscard]] bool getCapsLockProperty() const;

        /** @brief Gets whether the num lock key is down during the current state. */
        [[nodiscard]] bool getNumLockProperty() const;

        /** @brief Returns whether either the left or right shift key is down during the current state. */
        [[nodiscard]] bool IsShiftDown() const;

        /** @brief Returns whether either the left or right control key is down during the current state. */
        [[nodiscard]] bool IsControlDown() const;

        /** @brief Returns whether either the left or right alt key is down during the current state. */
        [[nodiscard]] bool IsAltDown() const;

        /** @brief Returns whether the specified key is down during the current state. */
        [[nodiscard]] bool IsKeyDown(Keys key) const;

        /** @brief Returns whether the specified key is up during the current state. */
        [[nodiscard]] bool IsKeyUp(Keys key) const;

        /** @brief Gets the total number of keys down during the current state. */
        [[nodiscard]] int getPressedKeyCountProperty() const;

        /** @brief Returns all keys that are down during the current state. */
        [[nodiscard]] std::vector<Keys> GetPressedKeys() const;

        /** @brief Returns whether the given key was down during the previous state, but is now up. */
        [[nodiscard]] bool WasKeyReleased(Keys key) const;

        /** @brief Returns whether the given key was up during the previous state, but is now down. */
        [[nodiscard]] bool WasKeyPressed(Keys key) const;

        /** @brief Returns whether any key was pressed down on the previous state. */
        [[nodiscard]] bool WasAnyKeyJustDown() const;

    private:
        KeyboardState currentKeyboardState_;
        KeyboardState previousKeyboardState_;
    };
}
