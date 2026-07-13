// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/KeyboardListenerSettings.cs.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListenerSettings.hpp"

namespace CNA::Extended::Input::InputListeners
{
    class KeyboardListener;

    /** @brief Settings/factory object for constructing a configured KeyboardListener. */
    class KeyboardListenerSettings : public InputListenerSettings<KeyboardListener>
    {
    public:
        KeyboardListenerSettings();

        /** @brief Gets or sets whether holding a key down repeatedly raises KeyPressed/KeyTyped. */
        [[nodiscard]] bool getRepeatPressProperty() const { return repeatPress_; }
        void setRepeatPressProperty(bool value) { repeatPress_ = value; }

        /** @brief Gets or sets the delay, in milliseconds, before the first repeat fires. */
        [[nodiscard]] int getInitialDelayMillisecondsProperty() const { return initialDelayMilliseconds_; }
        void setInitialDelayMillisecondsProperty(int value) { initialDelayMilliseconds_ = value; }

        /** @brief Gets or sets the delay, in milliseconds, between subsequent repeats. */
        [[nodiscard]] int getRepeatDelayMillisecondsProperty() const { return repeatDelayMilliseconds_; }
        void setRepeatDelayMillisecondsProperty(int value) { repeatDelayMilliseconds_ = value; }

        [[nodiscard]] std::unique_ptr<KeyboardListener> CreateListener() const override;

    private:
        bool repeatPress_ = true;
        int initialDelayMilliseconds_ = 800;
        int repeatDelayMilliseconds_ = 50;
    };
}
