// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/InputListener.cs. A simple abstract base
// with one pure virtual Update(GameTime&) -- unlike GameTimer (Phase 3's Timers module), this
// does NOT implement IUpdateable (no Enabled/UpdateOrder/EnabledChanged plumbing upstream),
// matching upstream's own minimal shape exactly.
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief Abstract base for input listeners (keyboard, mouse, gamepad, touch), each polling and diffing device state each Update(). */
    class InputListener
    {
    public:
        virtual ~InputListener() = default;

        /** @brief Polls the current device state and raises any resulting events. */
        virtual void Update(GameTime& gameTime) = 0;

    protected:
        InputListener() = default;
    };
}
