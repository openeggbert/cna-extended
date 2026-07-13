// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/BitArrayExtensionsTests.cs.
#include "CNA/Extended/ECS/BitArrayExtensions.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::ECS
{
    TEST(BitArrayExtensionsTests, BitArrayIsEmpty)
    {
        EXPECT_TRUE(IsEmpty(BitArray(1)));
        EXPECT_FALSE(IsEmpty(BitArray(std::vector<bool>{true})));
        EXPECT_TRUE(IsEmpty(BitArray(std::vector<bool>{false})));

        BitArray bitArray(std::vector<bool>{true});
        bitArray.Set(0, false);
        EXPECT_TRUE(IsEmpty(bitArray));
    }
}
