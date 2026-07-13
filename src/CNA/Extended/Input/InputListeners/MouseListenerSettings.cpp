// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/MouseListenerSettings.hpp"

#include "CNA/Extended/Input/InputListeners/MouseListener.hpp"

namespace CNA::Extended::Input::InputListeners
{
    MouseListenerSettings::MouseListenerSettings() = default;

    std::unique_ptr<MouseListener> MouseListenerSettings::CreateListener() const
    {
        return std::make_unique<MouseListener>(*this);
    }
}
