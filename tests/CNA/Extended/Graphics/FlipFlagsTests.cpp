// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for FlipFlags. Fresh tests below.
#include "CNA/Extended/Graphics/FlipFlags.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Graphics
{
    TEST(FlipFlagsTests, OrCombinesFlags)
    {
        const FlipFlags combined = FlipFlags::FlipHorizontally | FlipFlags::FlipVertically;
        EXPECT_EQ(combined & FlipFlags::FlipHorizontally, FlipFlags::FlipHorizontally);
        EXPECT_EQ(combined & FlipFlags::FlipVertically, FlipFlags::FlipVertically);
        EXPECT_EQ(combined & FlipFlags::FlipDiagonally, FlipFlags::None);
    }

    TEST(FlipFlagsTests, OrAssignAccumulatesFlags)
    {
        FlipFlags flags = FlipFlags::None;
        flags |= FlipFlags::FlipDiagonally;
        flags |= FlipFlags::FlipHorizontally;

        EXPECT_EQ(flags & FlipFlags::FlipDiagonally, FlipFlags::FlipDiagonally);
        EXPECT_EQ(flags & FlipFlags::FlipHorizontally, FlipFlags::FlipHorizontally);
        EXPECT_EQ(flags & FlipFlags::FlipVertically, FlipFlags::None);
    }
}
