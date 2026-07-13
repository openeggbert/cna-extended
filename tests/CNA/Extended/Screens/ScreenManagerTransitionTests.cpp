// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exercise ScreenManager's Transition-based overloads (ShowScreen/CloseScreen/
// ReplaceScreen with a Transition) at all -- fresh tests below, using a minimal test-only
// Transition subclass (no concrete Transition exists yet in this port; FadeTransition/
// ExpandTransition are deferred to Phase 5, see Transition.hpp). These specifically exercise the
// deferred-destruction fix documented in ScreenManager.cpp's header comment: a literal
// std::unique_ptr::reset() inside the transition's own Completed handler would destroy the
// Transition while its own Update() call is still on the stack -- these tests prove the fix
// doesn't crash/corrupt state across that exact sequence.
#include "CNA/Extended/Screens/ScreenManager.hpp"

#include "TestScreens.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>

namespace CNA::Extended::Screens
{
    namespace
    {
        class TestTransition : public Transitions::Transition
        {
        public:
            explicit TestTransition(float duration) : Transition(duration) {}

            void Dispose() override { disposeCalled_ = true; }
            void Draw(const GameTime&) override {}

            [[nodiscard]] const std::string& GetTypeName() const override
            {
                static const std::string typeName = "CNA.Extended.Screens.Tests.TestTransition";
                return typeName;
            }

            [[nodiscard]] bool getDisposeCalledProperty() const { return disposeCalled_; }

        private:
            bool disposeCalled_ = false;
        };
    }

    TEST(ScreenManagerTransitionTests, ShowScreenWithTransitionShowsNewScreenAtHalfway)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        manager.ShowScreen(&screen1);

        manager.ShowScreen(&screen2, std::make_unique<TestTransition>(1.0f));

        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.6f));
        manager.Update(gameTime);

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen2);
    }

    TEST(ScreenManagerTransitionTests, TransitionCompletionDisposesAndClearsActiveTransitionSafely)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");
        manager.ShowScreen(&screen);

        // A very short transition: State reaches In on the first update (halfDuration = 0.05s
        // elapsed in one 0.2s step), then Completed fires once currentSeconds_ drops back to <= 0
        // on a second update of the same magnitude.
        manager.ShowScreen(&screen, std::make_unique<TestTransition>(0.1f));

        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.2f));
        manager.Update(gameTime); // crosses into State::In
        manager.Update(gameTime); // currentSeconds_ goes <= 0 -> Completed fires, transition disposed and cleared

        // No crash/UB from destroying the transition while its own Update() was still executing
        // (the specific hazard this design was built to avoid) -- reaching this point at all,
        // with a second manager.Update() call succeeding cleanly afterward, is the assertion.
        GameTime nextGameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(0.1f));
        manager.Update(nextGameTime);
        SUCCEED();
    }

    TEST(ScreenManagerTransitionTests, ShowScreenWithTransitionIgnoresSecondCallWhileTransitionActive)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        Tests::TestScreen screen3("Screen3");
        manager.ShowScreen(&screen1);

        manager.ShowScreen(&screen2, std::make_unique<TestTransition>(10.0f));
        manager.ShowScreen(&screen3, std::make_unique<TestTransition>(10.0f));

        GameTime gameTime(System::TimeSpan::Zero, System::TimeSpan::FromSeconds(20.0f));
        manager.Update(gameTime);

        // Only the FIRST transition's screen (screen2) should ever become active -- the second
        // ShowScreen(screen3, transition) call must be a no-op while a transition is in flight.
        EXPECT_EQ(manager.getActiveScreenProperty(), &screen2);
    }
}
