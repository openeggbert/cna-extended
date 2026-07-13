// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/TouchListenerSettings.hpp"

#include "CNA/Extended/Input/InputListeners/TouchListener.hpp"

namespace CNA::Extended::Input::InputListeners
{
    TouchListenerSettings::TouchListenerSettings() = default;

    std::unique_ptr<TouchListener> TouchListenerSettings::CreateListener() const
    {
        return std::make_unique<TouchListener>(*this);
    }
}
