// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/KeyboardListener.cs. Upstream's
// `_keysValues = Enum.GetValues(typeof(Keys))` (iterating every possible Keys value each frame)
// has no C++ reflection equivalent -- ported as a static const std::array<Keys, 160> literal
// listing every CNA::Microsoft::Xna::Framework::Input::Keys enumerator, generated once from
// Keys.hpp's actual declarations (not hand-guessed), defined in the .cpp.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListener.hpp"
#include "CNA/Extended/Input/InputListeners/KeyboardEventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"
#include "System/TimeSpan.hpp"

namespace CNA::Extended::Input::InputListeners
{
    class KeyboardListenerSettings;

    /** @brief Polls keyboard state each Update() and raises KeyPressed/KeyReleased/KeyTyped events for state changes. */
    class KeyboardListener : public System::Object, public InputListener
    {
    public:
        /** @brief Raised when a key is pressed (initially, or via repeat) and maps to a printable character. */
        System::EventHandler<KeyboardEventArgs> KeyTyped;

        /** @brief Raised when a key transitions from up to down, or (if RepeatPress) repeats while held. */
        System::EventHandler<KeyboardEventArgs> KeyPressed;

        /** @brief Raised when a key transitions from down to up. */
        System::EventHandler<KeyboardEventArgs> KeyReleased;

        KeyboardListener();
        explicit KeyboardListener(const KeyboardListenerSettings& settings);

        /** @brief Gets whether holding a key down repeatedly raises KeyPressed/KeyTyped. */
        [[nodiscard]] bool getRepeatPressProperty() const { return repeatPress_; }

        /** @brief Gets the delay, in milliseconds, before the first repeat fires. */
        [[nodiscard]] int getInitialDelayProperty() const { return initialDelay_; }

        /** @brief Gets the delay, in milliseconds, between subsequent repeats. */
        [[nodiscard]] int getRepeatDelayProperty() const { return repeatDelay_; }

        void Update(GameTime& gameTime) override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        void RaisePressedEvents(GameTime& gameTime, const KeyboardState& currentState);
        void RaiseReleasedEvents(const KeyboardState& currentState);
        void RaiseRepeatEvents(GameTime& gameTime, const KeyboardState& currentState);

        bool repeatPress_;
        int initialDelay_;
        int repeatDelay_;

        bool isInitial_ = false;
        System::TimeSpan lastPressTime_;
        Keys previousKey_ = Keys::None;
        KeyboardState previousState_;
    };
}
