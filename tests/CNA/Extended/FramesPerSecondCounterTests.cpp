// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for FramesPerSecondCounter.cs. Fresh tests below.
#include "CNA/Extended/FramesPerSecondCounter.hpp"

#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using System::TimeSpan;

    TEST(FramesPerSecondCounterTests, FramesPerSecondStartsAtZero)
    {
        const FramesPerSecondCounter counter;
        EXPECT_EQ(counter.getFramesPerSecondProperty(), 0);
    }

    TEST(FramesPerSecondCounterTests, FirstUpdateAlwaysLatchesRegardlessOfElapsedTime)
    {
        // The internal timer starts already at one full second (matches upstream's
        // `_timer = _oneSecondTimeSpan;` field initializer), so the very first Update() call
        // always crosses the one-second threshold immediately, no matter how little time has
        // actually elapsed.
        FramesPerSecondCounter counter;
        GameTime gameTime(TimeSpan::FromSeconds(0.0), TimeSpan::FromSeconds(0.001));

        counter.Draw(gameTime);
        counter.Update(gameTime);

        EXPECT_EQ(counter.getFramesPerSecondProperty(), 1);
    }

    TEST(FramesPerSecondCounterTests, SubsequentLatchRequiresAFullSecondOfAccumulatedElapsedTime)
    {
        FramesPerSecondCounter counter;
        GameTime gameTime(TimeSpan::FromSeconds(0.0), TimeSpan::FromSeconds(0.1));

        // First Update() always latches immediately (see the test above); reset back to a clean
        // baseline before testing the "normal" latch-after-a-second behavior.
        counter.Draw(gameTime);
        counter.Update(gameTime);

        for (int i = 0; i < 5; i++)
        {
            counter.Draw(gameTime);
            counter.Update(gameTime);
        }

        // Only 0.5s has accumulated since the reset; FramesPerSecond hasn't latched again yet.
        EXPECT_EQ(counter.getFramesPerSecondProperty(), 1);
    }

    TEST(FramesPerSecondCounterTests, FramesPerSecondLatchesAfterOneSecondElapses)
    {
        FramesPerSecondCounter counter;
        GameTime gameTime(TimeSpan::FromSeconds(0.0), TimeSpan::FromSeconds(0.2));

        // 6 * 0.2s = 1.2s, crossing the one-second threshold; 5 Draw() calls happened by then.
        for (int i = 0; i < 6; i++)
        {
            counter.Draw(gameTime);
            counter.Update(gameTime);
        }

        EXPECT_EQ(counter.getFramesPerSecondProperty(), 5);
    }

    TEST(FramesPerSecondCounterTests, EnabledSetterRaisesEnabledChangedOnlyWhenValueChanges)
    {
        FramesPerSecondCounter counter;
        int raiseCount = 0;
        counter.EnabledChanged.Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        counter.setEnabledProperty(true);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_TRUE(counter.getEnabledProperty());

        counter.setEnabledProperty(true);
        EXPECT_EQ(raiseCount, 1);

        counter.setEnabledProperty(false);
        EXPECT_EQ(raiseCount, 2);
    }

    TEST(FramesPerSecondCounterTests, UpdateOrderSetterUpdatesTheValue)
    {
        FramesPerSecondCounter counter;
        counter.setUpdateOrderProperty(5);
        EXPECT_EQ(counter.getUpdateOrderProperty(), 5);
    }

    // Regression test for the upstream bug documented in FramesPerSecondCounter.hpp: the
    // UpdateOrder setter raises EnabledChanged instead of UpdateOrderChanged. Preserved
    // faithfully, not "fixed" -- see the header comment for the full explanation.
    TEST(FramesPerSecondCounterTests, UpdateOrderSetterRaisesEnabledChangedNotUpdateOrderChangedReproducesKnownUpstreamBug)
    {
        FramesPerSecondCounter counter;
        int enabledChangedCount = 0;
        int updateOrderChangedCount = 0;
        counter.EnabledChanged.Add([&enabledChangedCount](System::Object*, const System::EventArgs&) { enabledChangedCount++; });
        counter.UpdateOrderChanged.Add([&updateOrderChangedCount](System::Object*, const System::EventArgs&) { updateOrderChangedCount++; });

        counter.setUpdateOrderProperty(3);

        EXPECT_EQ(enabledChangedCount, 1) << "Matches upstream's bug: EnabledChanged fires on an UpdateOrder change.";
        EXPECT_EQ(updateOrderChangedCount, 0)
            << "If this starts failing, upstream's bug may have been fixed and this port's fidelity note should be revisited.";
    }

    TEST(FramesPerSecondCounterTests, GetEnabledChangedEventReturnsTheSameEventInstance)
    {
        FramesPerSecondCounter counter;
        int raiseCount = 0;
        counter.getEnabledChangedEvent().Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        counter.setEnabledProperty(true);
        EXPECT_EQ(raiseCount, 1);
    }

    TEST(FramesPerSecondCounterTests, GetUpdateOrderChangedEventReturnsTheUpdateOrderChangedMember)
    {
        FramesPerSecondCounter counter;
        EXPECT_EQ(&counter.getUpdateOrderChangedEvent(), &counter.UpdateOrderChanged);
    }
}
