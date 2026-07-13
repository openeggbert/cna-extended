// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream test files exist for the Input module. Fresh tests below, covering default values
// and CreateListener() for each XListenerSettings type.
#include "CNA/Extended/Input/InputListeners/GamePadListener.hpp"
#include "CNA/Extended/Input/InputListeners/GamePadListenerSettings.hpp"
#include "CNA/Extended/Input/InputListeners/KeyboardListener.hpp"
#include "CNA/Extended/Input/InputListeners/KeyboardListenerSettings.hpp"
#include "CNA/Extended/Input/InputListeners/MouseListener.hpp"
#include "CNA/Extended/Input/InputListeners/MouseListenerSettings.hpp"
#include "CNA/Extended/Input/InputListeners/TouchListener.hpp"
#include "CNA/Extended/Input/InputListeners/TouchListenerSettings.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Input::InputListeners
{
    TEST(KeyboardListenerSettingsTests, DefaultsMatchUpstream)
    {
        const KeyboardListenerSettings settings;
        EXPECT_TRUE(settings.getRepeatPressProperty());
        EXPECT_EQ(settings.getInitialDelayMillisecondsProperty(), 800);
        EXPECT_EQ(settings.getRepeatDelayMillisecondsProperty(), 50);
    }

    TEST(KeyboardListenerSettingsTests, CreateListenerAppliesSettings)
    {
        KeyboardListenerSettings settings;
        settings.setRepeatPressProperty(false);
        settings.setInitialDelayMillisecondsProperty(100);
        settings.setRepeatDelayMillisecondsProperty(10);

        const std::unique_ptr<KeyboardListener> listener = settings.CreateListener();
        EXPECT_FALSE(listener->getRepeatPressProperty());
        EXPECT_EQ(listener->getInitialDelayProperty(), 100);
        EXPECT_EQ(listener->getRepeatDelayProperty(), 10);
    }

    TEST(MouseListenerSettingsTests, DefaultsMatchUpstream)
    {
        const MouseListenerSettings settings;
        EXPECT_EQ(settings.getDoubleClickMillisecondsProperty(), 500);
        EXPECT_EQ(settings.getDragThresholdProperty(), 2);
        EXPECT_EQ(settings.getViewportAdapterProperty(), nullptr);
    }

    TEST(MouseListenerSettingsTests, CreateListenerAppliesSettings)
    {
        MouseListenerSettings settings;
        settings.setDoubleClickMillisecondsProperty(250);
        settings.setDragThresholdProperty(5);

        const std::unique_ptr<MouseListener> listener = settings.CreateListener();
        EXPECT_EQ(listener->getDoubleClickMillisecondsProperty(), 250);
        EXPECT_EQ(listener->getDragThresholdProperty(), 5);
    }

    TEST(GamePadListenerSettingsTests, DefaultsMatchUpstream)
    {
        const GamePadListenerSettings settings;
        EXPECT_EQ(settings.getPlayerIndexProperty(), Microsoft::Xna::Framework::PlayerIndex::One);
        EXPECT_TRUE(settings.getVibrationEnabledProperty());
        EXPECT_FLOAT_EQ(settings.getVibrationStrengthLeftProperty(), 1.0f);
        EXPECT_FLOAT_EQ(settings.getVibrationStrengthRightProperty(), 1.0f);
        EXPECT_FLOAT_EQ(settings.getTriggerDownTresholdProperty(), 0.15f);
        EXPECT_FLOAT_EQ(settings.getThumbstickDownTresholdProperty(), 0.5f);
        EXPECT_EQ(settings.getRepeatInitialDelayProperty(), 500);
        EXPECT_EQ(settings.getRepeatDelayProperty(), 50);
    }

    TEST(GamePadListenerSettingsTests, CreateListenerAppliesSettings)
    {
        const GamePadListenerSettings settings(Microsoft::Xna::Framework::PlayerIndex::Two, false);
        const std::unique_ptr<GamePadListener> listener = settings.CreateListener();

        EXPECT_EQ(listener->getPlayerIndexProperty(), Microsoft::Xna::Framework::PlayerIndex::Two);
        EXPECT_FALSE(listener->getVibrationEnabledProperty());
    }

    TEST(TouchListenerSettingsTests, DefaultViewportAdapterIsNull)
    {
        const TouchListenerSettings settings;
        EXPECT_EQ(settings.getViewportAdapterProperty(), nullptr);
    }

    TEST(TouchListenerSettingsTests, CreateListenerAppliesSettings)
    {
        const TouchListenerSettings settings;
        const std::unique_ptr<TouchListener> listener = settings.CreateListener();
        EXPECT_EQ(listener->getViewportAdapterProperty(), nullptr);
    }
}
