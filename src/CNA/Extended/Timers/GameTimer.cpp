// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// *** REAL C#/C++ SEMANTIC DIFFERENCE, NOT A FIDELITY GAP ***
// Upstream's constructor calls Restart(), which calls Stop(), which calls the abstract OnStopped()
// -- in C#, a virtual call from a base constructor dispatches to the most-derived override even
// before the derived class's own constructor body runs (a well-known C# gotcha). In C++, calling a
// pure virtual function during base-class construction is undefined behavior (or a link error for
// a truly pure function with no definition) -- the vtable pointer is still the base class's during
// base construction. GameTimer's constructor therefore does NOT call Restart(); each concrete
// subclass (ContinuousClock, CountdownTimer) calls Restart() itself as the last step of its own
// constructor, once its own vtable is fully established. The observable end state after
// construction is identical to upstream (both ContinuousClock::OnStopped() and
// CountdownTimer::OnStopped() only touch base-class-provided state, never anything the derived
// class's own constructor body would have set), so this is a construction-mechanism difference
// only, not a behavior difference.
#include "CNA/Extended/Timers/GameTimer.hpp"

namespace CNA::Extended::Timers
{
    GameTimer::GameTimer(double intervalSeconds) : GameTimer(System::TimeSpan::FromSeconds(intervalSeconds))
    {
    }

    GameTimer::GameTimer(const System::TimeSpan& interval) : interval_(interval)
    {
    }

    void GameTimer::setEnabledProperty(bool value)
    {
        if (enabled_ == value)
        {
            return;
        }
        enabled_ = value;
        EnabledChanged.Raise(this, System::EventArgs::Empty);
    }

    void GameTimer::setUpdateOrderProperty(SharpRuntime::intcs value)
    {
        if (updateOrder_ == value)
        {
            return;
        }
        updateOrder_ = value;
        EnabledChanged.Raise(this, System::EventArgs::Empty);
    }

    void GameTimer::Update(GameTime& gameTime)
    {
        if (state_ != TimerState::Started)
        {
            return;
        }

        currentTime_ = currentTime_ + gameTime.getElapsedGameTimeProperty();
        OnUpdate(gameTime);
    }

    void GameTimer::Start()
    {
        state_ = TimerState::Started;
        Started.Raise(this, System::EventArgs::Empty);
    }

    void GameTimer::Stop()
    {
        state_ = TimerState::Stopped;
        currentTime_ = System::TimeSpan::Zero;
        OnStopped();
        Stopped.Raise(this, System::EventArgs::Empty);
    }

    void GameTimer::Restart()
    {
        Stop();
        Start();
    }

    void GameTimer::Pause()
    {
        state_ = TimerState::Paused;
        Paused.Raise(this, System::EventArgs::Empty);
    }
}
