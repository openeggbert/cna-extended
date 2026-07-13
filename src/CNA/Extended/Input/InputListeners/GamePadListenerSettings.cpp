// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/GamePadListenerSettings.hpp"

#include "CNA/Extended/Input/InputListeners/GamePadListener.hpp"

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::PlayerIndex;

    GamePadListenerSettings::GamePadListenerSettings() : GamePadListenerSettings(PlayerIndex::One)
    {
    }

    GamePadListenerSettings::GamePadListenerSettings(
        PlayerIndex playerIndex, bool vibrationEnabled, float vibrationStrengthLeft, float vibrationStrengthRight)
        : playerIndex_(playerIndex),
          vibrationEnabled_(vibrationEnabled),
          vibrationStrengthLeft_(vibrationStrengthLeft),
          vibrationStrengthRight_(vibrationStrengthRight)
    {
    }

    std::unique_ptr<GamePadListener> GamePadListenerSettings::CreateListener() const
    {
        return std::make_unique<GamePadListener>(*this);
    }
}
