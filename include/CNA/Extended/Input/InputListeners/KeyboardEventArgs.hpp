// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/KeyboardEventArgs.cs. `char? Character`
// -> `std::optional<char>`, matching this project's established nullable-value-type convention.
#pragma once

#include "CNA/Extended/Input/InputListeners/KeyboardModifiers.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "System/EventArgs.hpp"

#include <optional>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Input::KeyboardState;
    using Microsoft::Xna::Framework::Input::Keys;

    /** @brief Carries data for a KeyboardListener key event: which key, and which modifiers were held. */
    class KeyboardEventArgs : public System::EventArgs
    {
    public:
        /**
         * @brief Initializes a new KeyboardEventArgs, deriving Modifiers from the given KeyboardState.
         * @param key The key this event concerns.
         * @param keyboardState The keyboard state at the time of the event, used to derive Modifiers.
         */
        KeyboardEventArgs(Keys key, const KeyboardState& keyboardState);

        /** @brief Gets the key this event concerns. */
        [[nodiscard]] Keys getKeyProperty() const { return Key_; }

        /** @brief Gets the modifier keys (Control/Shift/Alt) held at the time of the event. */
        [[nodiscard]] KeyboardModifiers getModifiersProperty() const { return Modifiers_; }

        /** @brief Gets the printable character this key represents given the active modifiers, or std::nullopt if it has none. */
        [[nodiscard]] std::optional<char> getCharacterProperty() const;

    private:
        static std::optional<char> ToChar(Keys key, KeyboardModifiers modifiers);

        Keys Key_;
        KeyboardModifiers Modifiers_;
    };
}
