// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/KeyboardListenerSettings.hpp"

#include "CNA/Extended/Input/InputListeners/KeyboardListener.hpp"

namespace CNA::Extended::Input::InputListeners
{
    KeyboardListenerSettings::KeyboardListenerSettings() = default;

    std::unique_ptr<KeyboardListener> KeyboardListenerSettings::CreateListener() const
    {
        return std::make_unique<KeyboardListener>(*this);
    }
}
