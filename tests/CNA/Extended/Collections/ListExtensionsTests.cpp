// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for ListExtensions.cs. Fresh tests below.
#include "CNA/Extended/Collections/ListExtensions.hpp"

#include <algorithm>
#include <gtest/gtest.h>

namespace CNA::Extended::Collections
{
    TEST(ListExtensionsTests, ShuffleReturnsReferenceToTheSameList)
    {
        std::vector<int> list = {1, 2, 3, 4, 5};
        System::Random random(1234);

        std::vector<int>& result = Shuffle(list, random);
        EXPECT_EQ(&result, &list);
    }

    TEST(ListExtensionsTests, ShufflePreservesAllElements)
    {
        std::vector<int> list = {1, 2, 3, 4, 5, 6, 7, 8};
        std::vector<int> original = list;
        System::Random random(42);

        Shuffle(list, random);

        std::vector<int> sortedShuffled = list;
        std::vector<int> sortedOriginal = original;
        std::sort(sortedShuffled.begin(), sortedShuffled.end());
        std::sort(sortedOriginal.begin(), sortedOriginal.end());
        EXPECT_EQ(sortedShuffled, sortedOriginal);
    }

    TEST(ListExtensionsTests, ShuffleWithSameSeedIsDeterministic)
    {
        std::vector<int> listA = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<int> listB = listA;

        System::Random randomA(777);
        System::Random randomB(777);

        Shuffle(listA, randomA);
        Shuffle(listB, randomB);

        EXPECT_EQ(listA, listB);
    }

    TEST(ListExtensionsTests, ShuffleOfSingleElementListIsANoOp)
    {
        std::vector<int> list = {42};
        System::Random random(1);

        Shuffle(list, random);

        ASSERT_EQ(list.size(), 1u);
        EXPECT_EQ(list[0], 42);
    }

    TEST(ListExtensionsTests, ShuffleOfEmptyListIsANoOp)
    {
        std::vector<int> list;
        System::Random random(1);

        Shuffle(list, random);

        EXPECT_TRUE(list.empty());
    }
}
