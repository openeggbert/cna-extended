// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for CyclicalList.cs (it's an internal
// implementation detail of Triangulator). These tests cover the cyclical indexer, Add/Remove/
// RemoveAt/Contains/IndexOf, and iteration directly.
#include "CNA/Extended/Triangulation/CyclicalList.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    TEST(CyclicalListTests, IndexerWrapsPositiveOutOfRange)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        EXPECT_EQ(list.getItem(3), 10);
        EXPECT_EQ(list.getItem(4), 20);
    }

    TEST(CyclicalListTests, IndexerWrapsNegative)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        EXPECT_EQ(list.getItem(-1), 30);
        EXPECT_EQ(list.getItem(-2), 20);
    }

    TEST(CyclicalListTests, RemoveAtRemovesTheCyclicallyIndexedItem)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        list.RemoveAt(3); // wraps to index 0, i.e. value 10
        EXPECT_EQ(list.getCountProperty(), 2);
        EXPECT_FALSE(list.Contains(10));
    }

    TEST(CyclicalListTests, ContainsAndIndexOf)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        EXPECT_TRUE(list.Contains(20));
        EXPECT_EQ(list.IndexOf(20), 1);
        EXPECT_EQ(list.IndexOf(99), -1);
    }

    TEST(CyclicalListTests, IterationVisitsAllItemsInOrder)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        std::vector<int> visited;
        for (const int item : list)
        {
            visited.push_back(item);
        }

        EXPECT_EQ(visited, (std::vector<int>{10, 20, 30}));
    }

    TEST(CyclicalListTests, ConstructFromCollection)
    {
        const std::vector<int> source = {1, 2, 3};
        const CyclicalList<int> list(source);
        EXPECT_EQ(list.getCountProperty(), 3);
        EXPECT_EQ(list[0], 1);
        EXPECT_EQ(list[2], 3);
    }

    TEST(CyclicalListTests, TrackedIndexerAndExplicitAccessorsUseWrappedSlot)
    {
        CyclicalList<int> list;
        list.Add(10);
        list.Add(20);
        list.Add(30);

        list[3] = 40;
        EXPECT_EQ(list.getItem(0), 40);

        list.setItem(-1, 50);
        EXPECT_EQ(list.getItem(2), 50);
    }
}
