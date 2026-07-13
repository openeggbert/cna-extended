// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Timers/ContinuousClock.cs: a GameTimer that raises Tick
// repeatedly every Interval. Restart() is called at the end of the constructor body here, not in
// the GameTimer base constructor -- see GameTimer.cpp's header comment for why (pure virtual call
// during base construction is unsafe in C++, unlike C#).
#pragma once

#include "CNA/Extended/Timers/GameTimer.hpp"

#include <string>

namespace CNA::Extended::Timers
{
    /** @brief A GameTimer that raises Tick repeatedly, once every Interval, while running. */
    class ContinuousClock : public GameTimer
    {
    public:
        /** @brief Raised each time Interval elapses. */
        System::EventHandler<System::EventArgs> Tick;

        explicit ContinuousClock(double intervalSeconds);
        explicit ContinuousClock(const System::TimeSpan& interval);

        /** @brief Gets the CurrentTime at which the next Tick will be raised. */
        [[nodiscard]] const System::TimeSpan& getNextTickTimeProperty() const { return nextTickTime_; }

        [[nodiscard]] const std::string& GetTypeName() const override;

    protected:
        void OnStopped() override;
        void OnUpdate(GameTime& gameTime) override;

    private:
        System::TimeSpan nextTickTime_;
    };
}
