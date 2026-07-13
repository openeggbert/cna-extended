// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Timers/ContinuousClock.hpp"

namespace CNA::Extended::Timers
{
    ContinuousClock::ContinuousClock(double intervalSeconds) : GameTimer(intervalSeconds)
    {
        Restart();
    }

    ContinuousClock::ContinuousClock(const System::TimeSpan& interval) : GameTimer(interval)
    {
        Restart();
    }

    void ContinuousClock::OnStopped()
    {
        nextTickTime_ = getCurrentTimeProperty() + getIntervalProperty();
    }

    void ContinuousClock::OnUpdate(GameTime&)
    {
        if (getCurrentTimeProperty() >= nextTickTime_)
        {
            nextTickTime_ = getCurrentTimeProperty() + getIntervalProperty();
            Tick.Raise(this, System::EventArgs::Empty);
        }
    }

    const std::string& ContinuousClock::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Timers.ContinuousClock";
        return typeName;
    }
}
