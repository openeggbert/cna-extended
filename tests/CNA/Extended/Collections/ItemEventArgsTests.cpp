// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for ItemEventArgs.cs. Fresh tests below.
#include "CNA/Extended/Collections/ItemEventArgs.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collections
{
    TEST(ItemEventArgsTests, ItemPropertyReturnsTheConstructedValue)
    {
        const ItemEventArgs<int> args(42);
        EXPECT_EQ(args.getItemProperty(), 42);
    }

    TEST(ItemEventArgsTests, IsAnEventArgs)
    {
        const ItemEventArgs<int> args(7);
        const System::EventArgs& base = args;
        (void)base;
        SUCCEED();
    }
}
