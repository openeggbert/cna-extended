// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/GamePadEventArgs.hpp"

namespace CNA::Extended::Input::InputListeners
{
    GamePadEventArgs::GamePadEventArgs(const GamePadState& previousState, const GamePadState& currentState,
        const System::TimeSpan& elapsedTime, PlayerIndex playerIndex, std::optional<Buttons> button, float triggerState,
        std::optional<Vector2> thumbStickState)
        : PlayerIndex_(playerIndex),
          PreviousState_(previousState),
          CurrentState_(currentState),
          ElapsedTime_(elapsedTime),
          TriggerState_(triggerState),
          ThumbStickState_(thumbStickState.value_or(Vector2::Zero))
    {
        if (button.has_value())
        {
            Button_ = button.value();
        }
    }
}
