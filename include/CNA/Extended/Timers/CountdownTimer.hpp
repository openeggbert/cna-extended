// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Timers/CountdownTimer.cs: a GameTimer that counts down from
// Interval to zero, raising TimeRemainingChanged each Update() and Completed once it reaches zero.
// Restart() is called at the end of the constructor body here, not in the GameTimer base
// constructor -- see GameTimer.cpp's header comment for why.
#pragma once

#include "CNA/Extended/Timers/GameTimer.hpp"

#include <string>

namespace CNA::Extended::Timers
{
    /** @brief A GameTimer that counts down from Interval to zero, then transitions to Completed. */
    class CountdownTimer : public GameTimer
    {
    public:
        /** @brief Raised each Update() while running. */
        System::EventHandler<System::EventArgs> TimeRemainingChanged;

        /** @brief Raised once when CurrentTime reaches Interval. */
        System::EventHandler<System::EventArgs> Completed;

        explicit CountdownTimer(double intervalSeconds);
        explicit CountdownTimer(const System::TimeSpan& interval);

        /** @brief Gets the time remaining until this timer completes. */
        [[nodiscard]] const System::TimeSpan& getTimeRemainingProperty() const { return timeRemaining_; }

        [[nodiscard]] const std::string& GetTypeName() const override;

    protected:
        void OnStopped() override;
        void OnUpdate(GameTime& gameTime) override;

    private:
        System::TimeSpan timeRemaining_;
    };
}
