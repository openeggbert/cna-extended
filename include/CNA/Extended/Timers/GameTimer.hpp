// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Timers/GameTimer.cs. Implements CNA's IUpdateable interface,
// following the same pattern established by FramesPerSecondCounter.hpp elsewhere in this project
// (EnabledChanged/UpdateOrderChanged as public System::EventHandler<System::EventArgs> members,
// getEnabledChangedEvent()/getUpdateOrderChangedEvent() returning references to them). Upstream's
// non-generic `event EventHandler` (Started/Stopped/Paused) is the same as `EventHandler<EventArgs>`
// in C# (the non-generic EventHandler delegate always carries EventArgs) -> ported as
// System::EventHandler<System::EventArgs>, matching the generic form used elsewhere.
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- flagged prominently, not silently fixed ***
// Upstream's `UpdateOrder` property setter raises `EnabledChanged?.Invoke(...)` instead of
// `UpdateOrderChanged?.Invoke(...)` -- the exact same copy-paste error already found and preserved
// in FramesPerSecondCounter.hpp (Phase 1). Reproduced exactly below: setUpdateOrderProperty()
// raises EnabledChanged, not UpdateOrderChanged. Per this project's "port 1:1, no simplification"
// requirement, this is not this port's call to fix.
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/IUpdateable.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"
#include "System/TimeSpan.hpp"

#include "CNA/Extended/Timers/TimerState.hpp"

namespace CNA::Extended::Timers
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::IUpdateable;

    /** @brief Abstract base for timers driven by GameTime::ElapsedGameTime, exposing Start/Stop/Pause lifecycle events. */
    class GameTimer : public System::Object, public IUpdateable
    {
    public:
        /** @brief Raised when Enabled changes. Also (see header comment) incorrectly raised by setUpdateOrderProperty(), matching upstream. */
        System::EventHandler<System::EventArgs> EnabledChanged;

        /** @brief Raised when UpdateOrder changes. Upstream never actually raises this event -- see header comment. */
        System::EventHandler<System::EventArgs> UpdateOrderChanged;

        /** @brief Raised by Start(). */
        System::EventHandler<System::EventArgs> Started;

        /** @brief Raised by Stop(). */
        System::EventHandler<System::EventArgs> Stopped;

        /** @brief Raised by Pause(). */
        System::EventHandler<System::EventArgs> Paused;

        ~GameTimer() override = default;

        [[nodiscard]] bool getEnabledProperty() const override { return enabled_; }
        /** @brief Sets whether this timer participates in the update loop. Raises EnabledChanged when the value changes. */
        void setEnabledProperty(bool value);

        [[nodiscard]] SharpRuntime::intcs getUpdateOrderProperty() const override { return updateOrder_; }
        /**
         * @brief Sets the update order. Raises EnabledChanged (not UpdateOrderChanged) when the
         * value changes -- see this file's header comment for why.
         */
        void setUpdateOrderProperty(SharpRuntime::intcs value);

        [[nodiscard]] System::EventHandler<System::EventArgs>& getEnabledChangedEvent() override { return EnabledChanged; }
        [[nodiscard]] System::EventHandler<System::EventArgs>& getUpdateOrderChangedEvent() override { return UpdateOrderChanged; }

        /** @brief Gets or sets the interval between ticks/completion. */
        [[nodiscard]] const System::TimeSpan& getIntervalProperty() const { return interval_; }
        void setIntervalProperty(const System::TimeSpan& value) { interval_ = value; }

        /** @brief Gets the elapsed time since this timer was last started. */
        [[nodiscard]] const System::TimeSpan& getCurrentTimeProperty() const { return currentTime_; }

        /** @brief Gets the current lifecycle state. */
        [[nodiscard]] TimerState getStateProperty() const { return state_; }

        /** @brief Advances CurrentTime by GameTime::ElapsedGameTime and invokes OnUpdate(), but only while State is Started. */
        void Update(GameTime& gameTime) override;

        /** @brief Transitions to the Started state and raises Started. */
        void Start();

        /** @brief Transitions to the Stopped state, resets CurrentTime to zero, calls OnStopped(), and raises Stopped. */
        void Stop();

        /** @brief Stops then starts this timer. */
        void Restart();

        /** @brief Transitions to the Paused state and raises Paused. */
        void Pause();

    protected:
        /**
         * @brief Initializes a new GameTimer with the specified interval, in seconds, and starts it.
         * @param intervalSeconds The interval between ticks/completion, in seconds.
         */
        explicit GameTimer(double intervalSeconds);

        /**
         * @brief Initializes a new GameTimer with the specified interval and starts it.
         * @param interval The interval between ticks/completion.
         */
        explicit GameTimer(const System::TimeSpan& interval);

        void setCurrentTimeProperty(const System::TimeSpan& value) { currentTime_ = value; }
        void setStateProperty(TimerState value) { state_ = value; }

        /** @brief Called by Stop() after CurrentTime has been reset to zero, before Stopped is raised. */
        virtual void OnStopped() = 0;

        /** @brief Called by Update() after CurrentTime has been advanced, while State is Started. */
        virtual void OnUpdate(GameTime& gameTime) = 0;

    private:
        bool enabled_ = false;
        SharpRuntime::intcs updateOrder_ = 0;
        System::TimeSpan interval_;
        System::TimeSpan currentTime_;
        TimerState state_ = TimerState::Stopped;
    };
}
