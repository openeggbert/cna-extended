// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Timers module. Fresh tests below, covering GameTimer's
// shared Start/Stop/Pause/Restart lifecycle via ContinuousClock (any concrete subclass exercises
// the same base logic), plus the documented UpdateOrder/EnabledChanged bug (matching the
// regression-test convention established by FramesPerSecondCounterTests.cpp in Phase 1).
#include "CNA/Extended/Timers/ContinuousClock.hpp"

#include "CNA/Extended/Timers/CountdownTimer.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Timers
{
    using Microsoft::Xna::Framework::GameTime;

    TEST(GameTimerTests, ConstructorStartsTheTimer)
    {
        const ContinuousClock clock(1.0);
        EXPECT_EQ(clock.getStateProperty(), TimerState::Started);
        EXPECT_EQ(clock.getCurrentTimeProperty(), System::TimeSpan::Zero);
    }

    TEST(GameTimerTests, UpdateAdvancesCurrentTimeOnlyWhileStarted)
    {
        ContinuousClock clock(10.0);
        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(1.0));

        clock.Update(gameTime);
        EXPECT_EQ(clock.getCurrentTimeProperty(), System::TimeSpan::FromSeconds(1.0));

        clock.Pause();
        clock.Update(gameTime);
        EXPECT_EQ(clock.getCurrentTimeProperty(), System::TimeSpan::FromSeconds(1.0));
    }

    TEST(GameTimerTests, StopResetsCurrentTimeAndRaisesStopped)
    {
        ContinuousClock clock(10.0);
        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(2.0));
        clock.Update(gameTime);

        bool stoppedRaised = false;
        clock.Stopped += [&stoppedRaised](System::Object*, const System::EventArgs&) { stoppedRaised = true; };

        clock.Stop();

        EXPECT_EQ(clock.getStateProperty(), TimerState::Stopped);
        EXPECT_EQ(clock.getCurrentTimeProperty(), System::TimeSpan::Zero);
        EXPECT_TRUE(stoppedRaised);
    }

    TEST(GameTimerTests, RestartStopsThenStarts)
    {
        ContinuousClock clock(10.0);
        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(3.0));
        clock.Update(gameTime);

        clock.Restart();

        EXPECT_EQ(clock.getStateProperty(), TimerState::Started);
        EXPECT_EQ(clock.getCurrentTimeProperty(), System::TimeSpan::Zero);
    }

    TEST(GameTimerTests, PauseTransitionsStateAndRaisesPaused)
    {
        ContinuousClock clock(10.0);

        bool pausedRaised = false;
        clock.Paused += [&pausedRaised](System::Object*, const System::EventArgs&) { pausedRaised = true; };

        clock.Pause();

        EXPECT_EQ(clock.getStateProperty(), TimerState::Paused);
        EXPECT_TRUE(pausedRaised);
    }

    TEST(GameTimerTests, SetEnabledRaisesEnabledChangedOnlyWhenValueChanges)
    {
        ContinuousClock clock(10.0);
        int raiseCount = 0;
        clock.EnabledChanged += [&raiseCount](System::Object*, const System::EventArgs&) { ++raiseCount; };

        clock.setEnabledProperty(true);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_TRUE(clock.getEnabledProperty());

        clock.setEnabledProperty(true);
        EXPECT_EQ(raiseCount, 1);
    }

    // *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- see GameTimer.hpp's header comment ***
    TEST(GameTimerTests, SetUpdateOrderRaisesEnabledChangedNotUpdateOrderChanged)
    {
        ContinuousClock clock(10.0);
        int enabledChangedCount = 0;
        int updateOrderChangedCount = 0;
        clock.EnabledChanged += [&enabledChangedCount](System::Object*, const System::EventArgs&) { ++enabledChangedCount; };
        clock.UpdateOrderChanged += [&updateOrderChangedCount](System::Object*, const System::EventArgs&) { ++updateOrderChangedCount; };

        clock.setUpdateOrderProperty(5);

        EXPECT_EQ(clock.getUpdateOrderProperty(), 5);
        EXPECT_EQ(enabledChangedCount, 1);
        EXPECT_EQ(updateOrderChangedCount, 0);
    }

    TEST(ContinuousClockTests, TicksOncePerIntervalAndAdvancesNextTickTime)
    {
        ContinuousClock clock(1.0);
        int tickCount = 0;
        clock.Tick += [&tickCount](System::Object*, const System::EventArgs&) { ++tickCount; };

        GameTime halfSecond(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.5));
        clock.Update(halfSecond);
        EXPECT_EQ(tickCount, 0);

        clock.Update(halfSecond);
        EXPECT_EQ(tickCount, 1);
        EXPECT_EQ(clock.getNextTickTimeProperty(), System::TimeSpan::FromSeconds(2.0));
    }

    TEST(CountdownTimerTests, RaisesTimeRemainingChangedEachUpdate)
    {
        CountdownTimer timer(2.0);
        int changedCount = 0;
        timer.TimeRemainingChanged += [&changedCount](System::Object*, const System::EventArgs&) { ++changedCount; };

        GameTime oneSecond(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(1.0));
        timer.Update(oneSecond);

        EXPECT_EQ(changedCount, 1);
        EXPECT_EQ(timer.getTimeRemainingProperty(), System::TimeSpan::FromSeconds(1.0));
    }

    TEST(CountdownTimerTests, CompletesWhenCurrentTimeReachesInterval)
    {
        CountdownTimer timer(1.0);
        bool completedRaised = false;
        timer.Completed += [&completedRaised](System::Object*, const System::EventArgs&) { completedRaised = true; };

        GameTime oneSecond(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(1.0));
        timer.Update(oneSecond);

        EXPECT_TRUE(completedRaised);
        EXPECT_EQ(timer.getStateProperty(), TimerState::Completed);
        EXPECT_EQ(timer.getCurrentTimeProperty(), System::TimeSpan::FromSeconds(1.0));
        EXPECT_EQ(timer.getTimeRemainingProperty(), System::TimeSpan::Zero);
    }

    TEST(CountdownTimerTests, DoesNotCompleteBeforeIntervalElapses)
    {
        CountdownTimer timer(2.0);
        bool completedRaised = false;
        timer.Completed += [&completedRaised](System::Object*, const System::EventArgs&) { completedRaised = true; };

        GameTime oneSecond(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(1.0));
        timer.Update(oneSecond);

        EXPECT_FALSE(completedRaised);
        EXPECT_EQ(timer.getStateProperty(), TimerState::Started);
    }
}
