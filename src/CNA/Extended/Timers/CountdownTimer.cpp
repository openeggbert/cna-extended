// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Timers/CountdownTimer.hpp"

namespace CNA::Extended::Timers
{
    CountdownTimer::CountdownTimer(double intervalSeconds) : GameTimer(intervalSeconds)
    {
        Restart();
    }

    CountdownTimer::CountdownTimer(const System::TimeSpan& interval) : GameTimer(interval)
    {
        Restart();
    }

    void CountdownTimer::OnStopped()
    {
        setCurrentTimeProperty(System::TimeSpan::Zero);
    }

    void CountdownTimer::OnUpdate(GameTime&)
    {
        timeRemaining_ = getIntervalProperty() - getCurrentTimeProperty();
        TimeRemainingChanged.Raise(this, System::EventArgs::Empty);

        if (getCurrentTimeProperty() >= getIntervalProperty())
        {
            setStateProperty(TimerState::Completed);
            setCurrentTimeProperty(getIntervalProperty());
            timeRemaining_ = System::TimeSpan::Zero;
            Completed.Raise(this, System::EventArgs::Empty);
        }
    }

    const std::string& CountdownTimer::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Timers.CountdownTimer";
        return typeName;
    }
}
