// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for IndexableCyclicalLinkedList.cs (it's an
// internal implementation detail of Triangulator). These tests cover the cyclical indexer,
// AddFirst/AddLast/AddAfter/Remove/RemoveAt/Contains/IndexOf directly.
#include "CNA/Extended/Triangulation/IndexableCyclicalLinkedList.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    TEST(IndexableCyclicalLinkedListTests, AddLastAndIndexerWrapPositive)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(10);
        list.AddLast(20);
        list.AddLast(30);

        EXPECT_EQ(list[0].getValueProperty(), 10);
        EXPECT_EQ(list[3].getValueProperty(), 10);
        EXPECT_EQ(list[4].getValueProperty(), 20);
    }

    TEST(IndexableCyclicalLinkedListTests, IndexerWrapsNegative)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(10);
        list.AddLast(20);
        list.AddLast(30);

        EXPECT_EQ(list[-1].getValueProperty(), 30);
    }

    TEST(IndexableCyclicalLinkedListTests, AddFirstInsertsAtHead)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(20);
        list.AddFirst(10);

        EXPECT_EQ(list[0].getValueProperty(), 10);
        EXPECT_EQ(list[1].getValueProperty(), 20);
    }

    TEST(IndexableCyclicalLinkedListTests, AddAfterInsertsInMiddle)
    {
        IndexableCyclicalLinkedList<int> list;
        const auto first = list.AddLast(10);
        list.AddLast(30);
        list.AddAfter(first, 20);

        EXPECT_EQ(list[0].getValueProperty(), 10);
        EXPECT_EQ(list[1].getValueProperty(), 20);
        EXPECT_EQ(list[2].getValueProperty(), 30);
    }

    TEST(IndexableCyclicalLinkedListTests, RemoveAtRemovesTheCyclicallyIndexedItem)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(10);
        list.AddLast(20);
        list.AddLast(30);

        list.RemoveAt(3); // wraps to index 0, i.e. value 10
        EXPECT_EQ(list.getCountProperty(), 2);
        EXPECT_FALSE(list.Contains(10));
    }

    TEST(IndexableCyclicalLinkedListTests, ContainsAndIndexOf)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(10);
        list.AddLast(20);
        list.AddLast(30);

        EXPECT_TRUE(list.Contains(20));
        EXPECT_EQ(list.IndexOf(20), 1);
        EXPECT_EQ(list.IndexOf(99), -1);
    }

    TEST(IndexableCyclicalLinkedListTests, ClearEmptiesTheList)
    {
        IndexableCyclicalLinkedList<int> list;
        list.AddLast(10);
        list.AddLast(20);
        list.Clear();

        EXPECT_EQ(list.getCountProperty(), 0);
    }
}
