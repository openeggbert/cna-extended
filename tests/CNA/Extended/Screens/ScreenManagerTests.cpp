// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Screens/ScreenManagerTests.cs.
// Screen* is non-owning in this port (see ScreenManager.hpp's header comment) -- tests own their
// TestScreen instances as local stack variables and pass raw pointers to the manager, matching the
// C# test's "caller retains the reference" pattern directly (just with an explicit `&`).
//
// Upstream's 4 "Screens_CachesResultsBetweenCalls"/"Screens_InvalidatesCacheOn*" tests check
// C#-specific reference identity (Same/NotSame) of the internally cached Reverse().ToArray()
// result -- a caching mechanism this port deliberately doesn't have (getScreensProperty() returns
// a direct const reference to the live, always-current screens_ vector; see ScreenManager.hpp's
// header comment). Those 4 tests have no meaningful translation (the "cache" here is just the live
// data itself, so identity is trivially always the same address, for a reason unrelated to what
// upstream's tests actually verify) -- replaced with content-correctness checks instead, since
// that's the behavior that actually matters to callers.
#include "CNA/Extended/Screens/ScreenManager.hpp"

#include "TestScreens.hpp"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace CNA::Extended::Screens
{
    TEST(ScreenManagerTests, ShowScreenFirstScreenBecomesActive)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");

        manager.ShowScreen(&screen);

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen);
        EXPECT_TRUE(screen.getIsActiveProperty());
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);
    }

    TEST(ScreenManagerTests, ShowScreenSecondScreenDeactivatesFirstAndBecomesActive)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen2);
        EXPECT_TRUE(screen2.getIsActiveProperty());
        EXPECT_FALSE(screen1.getIsActiveProperty());
        EXPECT_EQ(manager.getScreensProperty().size(), 2u);
    }

    TEST(ScreenManagerTests, ShowScreenSetsScreenManagerProperty)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");

        manager.ShowScreen(&screen);

        EXPECT_EQ(screen.getScreenManagerProperty(), &manager);
    }

    TEST(ScreenManagerTests, ShowScreenWithNullThrowsInvalidArgument)
    {
        ScreenManager manager;
        EXPECT_THROW(manager.ShowScreen(nullptr), std::invalid_argument);
    }

    TEST(ScreenManagerTests, CloseScreenRemovesActiveAndActivatesPrevious)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.CloseScreen();

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen1);
        EXPECT_TRUE(screen1.getIsActiveProperty());
        EXPECT_TRUE(screen2.getDisposeCalledProperty());
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);
    }

    TEST(ScreenManagerTests, CloseScreenOnLastScreenLeavesEmptyStack)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");
        manager.ShowScreen(&screen);

        manager.CloseScreen();

        EXPECT_EQ(manager.getActiveScreenProperty(), nullptr);
        EXPECT_TRUE(manager.getScreensProperty().empty());
        EXPECT_TRUE(screen.getDisposeCalledProperty());
    }

    TEST(ScreenManagerTests, CloseScreenOnEmptyStackDoesNotThrow)
    {
        ScreenManager manager;
        manager.CloseScreen();

        EXPECT_EQ(manager.getActiveScreenProperty(), nullptr);
        EXPECT_TRUE(manager.getScreensProperty().empty());
    }

    TEST(ScreenManagerTests, ReplaceScreenClosesActiveAndShowsNew)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        manager.ShowScreen(&screen1);

        manager.ReplaceScreen(&screen2);

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen2);
        EXPECT_TRUE(screen2.getIsActiveProperty());
        EXPECT_TRUE(screen1.getDisposeCalledProperty());
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);
    }

    TEST(ScreenManagerTests, ReplaceScreenOnEmptyStackJustShowsScreen)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");

        manager.ReplaceScreen(&screen);

        EXPECT_EQ(manager.getActiveScreenProperty(), &screen);
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);
    }

    TEST(ScreenManagerTests, ReplaceScreenWithNullThrowsInvalidArgument)
    {
        ScreenManager manager;
        EXPECT_THROW(manager.ReplaceScreen(nullptr), std::invalid_argument);
    }

    TEST(ScreenManagerTests, ClearScreensDisposesAllScreensAndClearsStack)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        Tests::TestScreen screen3("Screen3");

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);
        manager.ShowScreen(&screen3);

        manager.ClearScreens();

        EXPECT_EQ(manager.getActiveScreenProperty(), nullptr);
        EXPECT_TRUE(manager.getScreensProperty().empty());
        EXPECT_TRUE(screen1.getDisposeCalledProperty());
        EXPECT_TRUE(screen2.getDisposeCalledProperty());
        EXPECT_TRUE(screen3.getDisposeCalledProperty());
    }

    TEST(ScreenManagerTests, ClearScreensOnEmptyStackDoesNotThrow)
    {
        ScreenManager manager;
        manager.ClearScreens();
        EXPECT_EQ(manager.getActiveScreenProperty(), nullptr);
    }

    TEST(ScreenManagerTests, UpdateOnlyUpdatesActiveScreenByDefault)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        GameTime gameTime;

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.Update(gameTime);

        EXPECT_EQ(screen1.getUpdateCallCountProperty(), 0);
        EXPECT_EQ(screen2.getUpdateCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, UpdateUpdatesInactiveScreensWhenUpdateWhenInactiveIsTrue)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        screen1.setUpdateWhenInactiveProperty(true);
        Tests::TestScreen screen2("Screen2");
        GameTime gameTime;

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.Update(gameTime);

        EXPECT_EQ(screen1.getUpdateCallCountProperty(), 1);
        EXPECT_EQ(screen2.getUpdateCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, UpdateUpdatesScreensInBottomToTopOrder)
    {
        ScreenManager manager;
        std::vector<std::string> updateOrder;
        Tests::TestScreen screen1("Screen1");
        screen1.setUpdateWhenInactiveProperty(true);
        Tests::TestScreen screen2("Screen2");
        screen2.setUpdateWhenInactiveProperty(true);
        Tests::TestScreen screen3("Screen3");

        screen1.onUpdate = [&updateOrder](const std::string& name) { updateOrder.push_back(name); };
        screen2.onUpdate = [&updateOrder](const std::string& name) { updateOrder.push_back(name); };
        screen3.onUpdate = [&updateOrder](const std::string& name) { updateOrder.push_back(name); };

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);
        manager.ShowScreen(&screen3);

        GameTime gameTime;
        manager.Update(gameTime);

        ASSERT_EQ(updateOrder.size(), 3u);
        EXPECT_EQ(updateOrder[0], "Screen1");
        EXPECT_EQ(updateOrder[1], "Screen2");
        EXPECT_EQ(updateOrder[2], "Screen3");
    }

    TEST(ScreenManagerTests, UpdateOnEmptyStackDoesNotThrow)
    {
        ScreenManager manager;
        GameTime gameTime;
        manager.Update(gameTime);
    }

    TEST(ScreenManagerTests, DrawOnlyDrawsActiveScreenByDefault)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        GameTime gameTime;

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.Draw(gameTime);

        EXPECT_EQ(screen1.getDrawCallCountProperty(), 0);
        EXPECT_EQ(screen2.getDrawCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, DrawDrawsInactiveScreensWhenDrawWhenInactiveIsTrue)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        screen1.setDrawWhenInactiveProperty(true);
        Tests::TestScreen screen2("Screen2");
        GameTime gameTime;

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.Draw(gameTime);

        EXPECT_EQ(screen1.getDrawCallCountProperty(), 1);
        EXPECT_EQ(screen2.getDrawCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, DrawDrawsScreensInBottomToTopOrder)
    {
        ScreenManager manager;
        std::vector<std::string> drawOrder;
        Tests::TestScreen screen1("Screen1");
        screen1.setDrawWhenInactiveProperty(true);
        Tests::TestScreen screen2("Screen2");
        screen2.setDrawWhenInactiveProperty(true);
        Tests::TestScreen screen3("Screen3");

        screen1.onDraw = [&drawOrder](const std::string& name) { drawOrder.push_back(name); };
        screen2.onDraw = [&drawOrder](const std::string& name) { drawOrder.push_back(name); };
        screen3.onDraw = [&drawOrder](const std::string& name) { drawOrder.push_back(name); };

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);
        manager.ShowScreen(&screen3);

        GameTime gameTime;
        manager.Draw(gameTime);

        ASSERT_EQ(drawOrder.size(), 3u);
        EXPECT_EQ(drawOrder[0], "Screen1");
        EXPECT_EQ(drawOrder[1], "Screen2");
        EXPECT_EQ(drawOrder[2], "Screen3");
    }

    TEST(ScreenManagerTests, DrawOnEmptyStackDoesNotThrow)
    {
        ScreenManager manager;
        GameTime gameTime;
        manager.Draw(gameTime);
    }

    TEST(ScreenManagerTests, ShowScreenFirstScreenCallsOnActivated)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");

        manager.ShowScreen(&screen);

        EXPECT_EQ(screen.getActivatedCallCountProperty(), 1);
        EXPECT_EQ(screen.getDeactivatedCallCountProperty(), 0);
    }

    TEST(ScreenManagerTests, ShowScreenSecondScreenCallsOnDeactivatedOnFirstAndOnActivatedOnSecond)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        EXPECT_EQ(screen1.getActivatedCallCountProperty(), 1);
        EXPECT_EQ(screen1.getDeactivatedCallCountProperty(), 1);
        EXPECT_EQ(screen2.getActivatedCallCountProperty(), 1);
        EXPECT_EQ(screen2.getDeactivatedCallCountProperty(), 0);
    }

    TEST(ScreenManagerTests, CloseScreenCallsOnDeactivatedOnClosedScreenAndOnActivatedOnRevealed)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);

        manager.CloseScreen();

        EXPECT_EQ(screen2.getDeactivatedCallCountProperty(), 1);
        EXPECT_EQ(screen1.getActivatedCallCountProperty(), 2); // once on show, once on re-activation
    }

    TEST(ScreenManagerTests, CloseScreenLastScreenCallsOnDeactivatedButNotOnActivated)
    {
        ScreenManager manager;
        Tests::TestScreen screen("Screen1");
        manager.ShowScreen(&screen);

        manager.CloseScreen();

        EXPECT_EQ(screen.getDeactivatedCallCountProperty(), 1);
        EXPECT_EQ(screen.getActivatedCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, ClearScreensCallsOnDeactivatedOnlyOnActiveScreen)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        Tests::TestScreen screen3("Screen3");
        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);
        manager.ShowScreen(&screen3);

        const int screen1CountBefore = screen1.getDeactivatedCallCountProperty();
        const int screen2CountBefore = screen2.getDeactivatedCallCountProperty();

        manager.ClearScreens();

        EXPECT_EQ(screen1CountBefore, screen1.getDeactivatedCallCountProperty());
        EXPECT_EQ(screen2CountBefore, screen2.getDeactivatedCallCountProperty());
        EXPECT_EQ(screen3.getDeactivatedCallCountProperty(), 1);
    }

    TEST(ScreenManagerTests, ScreensReturnsScreensInBottomToTopOrder)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        Tests::TestScreen screen2("Screen2");
        Tests::TestScreen screen3("Screen3");

        manager.ShowScreen(&screen1);
        manager.ShowScreen(&screen2);
        manager.ShowScreen(&screen3);

        const std::vector<Screen*>& screens = manager.getScreensProperty();
        ASSERT_EQ(screens.size(), 3u);
        EXPECT_EQ(screens[0], &screen1);
        EXPECT_EQ(screens[1], &screen2);
        EXPECT_EQ(screens[2], &screen3);
    }

    TEST(ScreenManagerTests, ScreensReflectsContentAfterMutation)
    {
        ScreenManager manager;
        Tests::TestScreen screen1("Screen1");
        manager.ShowScreen(&screen1);
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);

        Tests::TestScreen screen2("Screen2");
        manager.ShowScreen(&screen2);
        EXPECT_EQ(manager.getScreensProperty().size(), 2u);

        manager.CloseScreen();
        EXPECT_EQ(manager.getScreensProperty().size(), 1u);

        manager.ClearScreens();
        EXPECT_TRUE(manager.getScreensProperty().empty());
    }
}
