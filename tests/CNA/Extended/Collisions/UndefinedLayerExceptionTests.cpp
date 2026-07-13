// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream UndefinedLayerExceptionTest.cs exists (only exercised indirectly, through
// CollisionWorld2D's own test suite, not yet ported). Fresh tests below.
#include "CNA/Extended/Collisions/UndefinedLayerException.hpp"

#include "System/ArgumentException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collisions
{
    TEST(UndefinedLayerExceptionTests, StoresLayerNameAndFormatsMessage)
    {
        const UndefinedLayerException ex("enemies");
        EXPECT_EQ(ex.getLayerNameProperty(), "enemies");
        EXPECT_STREQ(ex.what(), "Layer 'enemies' is not defined.");
    }

    TEST(UndefinedLayerExceptionTests, ThrowsOnEmptyLayerName)
    {
        EXPECT_THROW(UndefinedLayerException ex(""), System::ArgumentException);
    }

    TEST(UndefinedLayerExceptionTests, ThrowsOnWhitespaceLayerName)
    {
        EXPECT_THROW(UndefinedLayerException ex("   "), System::ArgumentException);
    }
}
