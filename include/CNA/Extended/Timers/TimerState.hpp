// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Timers/TimerState.cs. Upstream lives in the
// MonoGame.Extended.Timers namespace; ported into a matching CNA::Extended::Timers sub-namespace,
// per this project's "sub-namespaced per module" convention.
#pragma once

namespace CNA::Extended::Timers
{
    /** @brief Identifies the current lifecycle state of a GameTimer. */
    enum class TimerState
    {
        Started,
        Stopped,
        Paused,
        Completed
    };
}
