// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/Screens/GameScreenTests.cs.
#include "CNA/Extended/Screens/GameScreen.hpp"

#include "TestScreens.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::Screens
{
    TEST(GameScreenTests, ConstructorWithNullGameThrowsInvalidArgument)
    {
        EXPECT_THROW(Tests::TestGameScreen screen(nullptr), std::invalid_argument);
    }
}
