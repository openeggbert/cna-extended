// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for Pool.cs. Fresh tests below.
#include "CNA/Extended/Collections/Pool.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Collections
{
    namespace
    {
        struct PoolItem
        {
            int Value = 0;
        };
    }

    TEST(PoolTests, ObtainCreatesNewItemWhenPoolIsEmpty)
    {
        int createCount = 0;
        Pool<PoolItem> pool([&createCount] {
            createCount++;
            return new PoolItem();
        });

        PoolItem* item = pool.Obtain();
        EXPECT_NE(item, nullptr);
        EXPECT_EQ(createCount, 1);

        delete item;
    }

    TEST(PoolTests, FreeAndObtainReusesTheSameItem)
    {
        int createCount = 0;
        Pool<PoolItem> pool([&createCount] {
            createCount++;
            return new PoolItem();
        });

        PoolItem* item1 = pool.Obtain();
        pool.Free(item1);
        PoolItem* item2 = pool.Obtain();

        EXPECT_EQ(item1, item2);
        EXPECT_EQ(createCount, 1);

        delete item1;
    }

    TEST(PoolTests, FreeCallsResetItem)
    {
        int resetCount = 0;
        Pool<PoolItem> pool(
            [] { return new PoolItem(); }, [&resetCount](PoolItem*) { resetCount++; });

        PoolItem* item = pool.Obtain();
        pool.Free(item);

        EXPECT_EQ(resetCount, 1);

        delete item;
    }

    TEST(PoolTests, FreeCallsResetItemEvenWhenDiscardedPastMaximum)
    {
        // Matches upstream (see Pool.hpp's header comment): resetItem is called unconditionally,
        // even when the item is not actually kept because the pool is already at its maximum.
        int resetCount = 0;
        Pool<PoolItem> pool(
            [] { return new PoolItem(); }, [&resetCount](PoolItem*) { resetCount++; }, 16, 0);

        PoolItem* item = pool.Obtain();
        pool.Free(item);

        EXPECT_EQ(resetCount, 1);
        EXPECT_EQ(pool.getAvailableCountProperty(), 0);

        delete item;
    }

    TEST(PoolTests, FreeThrowsForNullItem)
    {
        Pool<PoolItem> pool([] { return new PoolItem(); });
        EXPECT_THROW(pool.Free(nullptr), std::invalid_argument);
    }

    TEST(PoolTests, MaximumLimitsAvailableCount)
    {
        Pool<PoolItem> pool(
            [] { return new PoolItem(); }, [](PoolItem*) {}, 16, 1);

        PoolItem* item1 = pool.Obtain();
        PoolItem* item2 = pool.Obtain();
        pool.Free(item1);
        pool.Free(item2);

        EXPECT_EQ(pool.getAvailableCountProperty(), 1);

        delete item1;
        delete item2;
    }

    TEST(PoolTests, ClearRemovesAllFreeItems)
    {
        Pool<PoolItem> pool([] { return new PoolItem(); });
        PoolItem* item = pool.Obtain();
        pool.Free(item);
        ASSERT_EQ(pool.getAvailableCountProperty(), 1);

        pool.Clear();
        EXPECT_EQ(pool.getAvailableCountProperty(), 0);

        delete item;
    }
}
