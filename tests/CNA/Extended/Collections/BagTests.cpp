// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended's only upstream test for Bag.cs (Bag_Enumeration_Does_Not_Allocate) is a
// C#-GC/boxing-allocation benchmark with no C++ equivalent concept -- not ported. All tests
// below are fresh, covering the actual correctness behavior.
#include "CNA/Extended/Collections/Bag.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collections
{
    TEST(BagTests, DefaultCapacityIsSixteen)
    {
        const Bag<int> bag;
        EXPECT_EQ(bag.getCapacityProperty(), 16u);
        EXPECT_EQ(bag.getCountProperty(), 0u);
        EXPECT_TRUE(bag.getIsEmptyProperty());
    }

    TEST(BagTests, ExplicitCapacityIsHonored)
    {
        const Bag<int> bag(4);
        EXPECT_EQ(bag.getCapacityProperty(), 4u);
    }

    TEST(BagTests, IndexerGetReturnsAddedElements)
    {
        Bag<int> bag;
        bag.Add(10);
        bag.Add(20);
        EXPECT_EQ(bag[0], 10);
        EXPECT_EQ(bag[1], 20);
    }

    TEST(BagTests, IndexerGetOutOfRangeReturnsDefaultRatherThanThrowing)
    {
        const Bag<int> bag(2);
        EXPECT_EQ(bag[100], 0);
    }

    TEST(BagTests, IndexerSetGrowsCapacityAndCount)
    {
        Bag<int> bag(2);
        bag.Set(5, 42);
        EXPECT_GE(bag.getCapacityProperty(), 6u);
        EXPECT_EQ(bag.getCountProperty(), 6u);
        EXPECT_EQ(bag[5], 42);
    }

    TEST(BagTests, IndexerSetWithinExistingCountDoesNotChangeCount)
    {
        Bag<int> bag;
        bag.Add(1);
        bag.Add(2);
        bag.Set(0, 99);
        EXPECT_EQ(bag.getCountProperty(), 2u);
        EXPECT_EQ(bag[0], 99);
    }

    TEST(BagTests, AddIncrementsCount)
    {
        Bag<int> bag;
        bag.Add(1);
        bag.Add(2);
        bag.Add(3);
        EXPECT_EQ(bag.getCountProperty(), 3u);
    }

    TEST(BagTests, AddRangeCopiesAllElements)
    {
        Bag<int> source;
        source.Add(1);
        source.Add(2);
        source.Add(3);

        Bag<int> destination;
        destination.Add(0);
        destination.AddRange(source);

        EXPECT_EQ(destination.getCountProperty(), 4u);
        EXPECT_EQ(destination[0], 0);
        EXPECT_EQ(destination[1], 1);
        EXPECT_EQ(destination[2], 2);
        EXPECT_EQ(destination[3], 3);
    }

    TEST(BagTests, ClearResetsCountToZero)
    {
        Bag<int> bag;
        bag.Add(1);
        bag.Add(2);
        bag.Clear();
        EXPECT_EQ(bag.getCountProperty(), 0u);
        EXPECT_TRUE(bag.getIsEmptyProperty());
    }

    TEST(BagTests, ClearOnEmptyBagIsANoOp)
    {
        Bag<int> bag;
        bag.Clear();
        EXPECT_EQ(bag.getCountProperty(), 0u);
    }

    TEST(BagTests, ContainsFindsAddedElement)
    {
        Bag<int> bag;
        bag.Add(1);
        bag.Add(2);
        bag.Add(3);
        EXPECT_TRUE(bag.Contains(2));
        EXPECT_FALSE(bag.Contains(99));
    }

    TEST(BagTests, RemoveAtSwapsWithLastRatherThanShifting)
    {
        Bag<int> bag;
        bag.Add(10);
        bag.Add(20);
        bag.Add(30);

        const int removed = bag.RemoveAt(0);

        EXPECT_EQ(removed, 10);
        EXPECT_EQ(bag.getCountProperty(), 2u);
        // Order is NOT preserved: the last element (30) is swapped into the removed slot.
        EXPECT_EQ(bag[0], 30);
        EXPECT_EQ(bag[1], 20);
    }

    TEST(BagTests, RemoveFindsAndSwapRemovesElement)
    {
        Bag<int> bag;
        bag.Add(10);
        bag.Add(20);
        bag.Add(30);

        const bool removed = bag.Remove(20);

        EXPECT_TRUE(removed);
        EXPECT_EQ(bag.getCountProperty(), 2u);
        EXPECT_EQ(bag[0], 10);
        EXPECT_EQ(bag[1], 30);
    }

    TEST(BagTests, RemoveReturnsFalseForAbsentElement)
    {
        Bag<int> bag;
        bag.Add(10);
        EXPECT_FALSE(bag.Remove(99));
        EXPECT_EQ(bag.getCountProperty(), 1u);
    }

    TEST(BagTests, RemoveAllRemovesEveryMatchingElement)
    {
        Bag<int> bag;
        bag.Add(1);
        bag.Add(2);
        bag.Add(3);
        bag.Add(4);

        Bag<int> toRemove;
        toRemove.Add(2);
        toRemove.Add(4);

        const bool result = bag.RemoveAll(toRemove);

        EXPECT_TRUE(result);
        EXPECT_EQ(bag.getCountProperty(), 2u);
        EXPECT_TRUE(bag.Contains(1));
        EXPECT_TRUE(bag.Contains(3));
        EXPECT_FALSE(bag.Contains(2));
        EXPECT_FALSE(bag.Contains(4));
    }

    TEST(BagTests, RemoveAllReturnsFalseWhenNothingRemoved)
    {
        Bag<int> bag;
        bag.Add(1);

        Bag<int> toRemove;
        toRemove.Add(99);

        EXPECT_FALSE(bag.RemoveAll(toRemove));
    }

    TEST(BagTests, CapacityGrowsByOnePointFiveTimesOrToRequiredSize)
    {
        Bag<int> bag(4);
        for (int i = 0; i < 5; i++)
        {
            bag.Add(i);
        }
        // 4 * 1.5 = 6, which satisfies the 5th add (index 4) without needing max().
        EXPECT_GE(bag.getCapacityProperty(), 5u);
        EXPECT_EQ(bag.getCountProperty(), 5u);
    }

    TEST(BagTests, IterationVisitsElementsInOrderUpToCount)
    {
        Bag<int> bag(2);
        bag.Add(1);
        bag.Add(2);
        bag.Add(3);

        std::vector<int> visited;
        for (const int value : bag)
        {
            visited.push_back(value);
        }

        EXPECT_EQ(visited, (std::vector<int>{1, 2, 3}));
    }

    TEST(BagTests, ConstIterationWorks)
    {
        Bag<int> bag;
        bag.Add(5);
        bag.Add(6);

        const Bag<int>& constBag = bag;
        std::vector<int> visited;
        for (const int value : constBag)
        {
            visited.push_back(value);
        }

        EXPECT_EQ(visited, (std::vector<int>{5, 6}));
    }
}
