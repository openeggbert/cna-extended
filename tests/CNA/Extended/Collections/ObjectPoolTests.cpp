// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Collections/ObjectPoolTests.cs
// (one test, 1:1 below: ObjectPool_ThrowsNullReferenceException_WhenAllItemsReturnedAndNewCalled
// -- the name is stale upstream, the test actually asserts no exception is thrown). Everything
// else is fresh, including regression tests for the severe self-referencing-node infinite-loop
// bug documented in ObjectPool.hpp's header comment.
//
// IMPORTANT: several tests below deliberately do NOT iterate (begin()/end()/range-based for) a
// pool in states where ObjectPool.hpp's header comment says iteration would hang forever (any
// pool whose current tail item was obtained via New() and never Return()ed). Those tests instead
// assert directly on getNextNodeProperty()/getPreviousNodeProperty(). Do not "simplify" those
// tests to use iteration -- doing so would hang the test process.
#include "CNA/Extended/Collections/ObjectPool.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace CNA::Extended::Collections
{
    namespace
    {
        class TestPoolable final : public IPoolable
        {
        public:
            std::function<void(IPoolable*)> ReturnAction;
            IPoolable* NextNode = nullptr;
            IPoolable* PreviousNode = nullptr;

            [[nodiscard]] IPoolable* getNextNodeProperty() const override { return NextNode; }
            void setNextNodeProperty(IPoolable* value) override { NextNode = value; }
            [[nodiscard]] IPoolable* getPreviousNodeProperty() const override { return PreviousNode; }
            void setPreviousNodeProperty(IPoolable* value) override { PreviousNode = value; }

            void Initialize(std::function<void(IPoolable*)> returnDelegate) override { ReturnAction = std::move(returnDelegate); }
            void Return() override { ReturnAction(this); }
        };
    }

    // Ported 1:1 from upstream's ObjectPool_ThrowsNullReferenceException_WhenAllItemsReturnedAndNewCalled.
    TEST(ObjectPoolTests, NoExceptionWhenAllItemsReturnedAndNewCalledAgain)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 2);

        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();

        item1->Return();
        item2->Return();

        TestPoolable* item3 = nullptr;
        EXPECT_NO_THROW(item3 = pool.New());
        EXPECT_EQ(item3, item1); // Reuses the front of the free list (item1, returned first).

        delete item1;
        delete item2;
    }

    TEST(ObjectPoolTests, NewCreatesInstancesUpToCapacity)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 3);

        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();
        TestPoolable* item3 = pool.New();

        EXPECT_NE(item1, nullptr);
        EXPECT_NE(item2, nullptr);
        EXPECT_NE(item3, nullptr);
        EXPECT_NE(item1, item2);
        EXPECT_NE(item2, item3);
        EXPECT_EQ(pool.getTotalCountProperty(), 3);
        EXPECT_EQ(pool.getInUseCountProperty(), 3);
        EXPECT_EQ(pool.getAvailableCountProperty(), 0);

        delete item1;
        delete item2;
        delete item3;
    }

    TEST(ObjectPoolTests, ReturnedItemIsReusedByNextNew)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 2);

        TestPoolable* item1 = pool.New();
        item1->Return();

        TestPoolable* item2 = pool.New();
        EXPECT_EQ(item1, item2);
        EXPECT_EQ(pool.getTotalCountProperty(), 1);

        delete item1;
    }

    TEST(ObjectPoolTests, ReturnNullPolicyReturnsNullPastCapacity)
    {
        // Matches upstream exactly: New() creates fresh instances while TotalCount <= Capacity
        // (note <=, not <), so with Capacity(1) it takes 3 calls -- not 2 -- before TotalCount(2)
        // finally exceeds Capacity(1) and the full-pool policy actually applies.
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 1, ObjectPoolIsFullPolicy::ReturnNull);

        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();
        TestPoolable* item3 = pool.New();

        EXPECT_NE(item1, nullptr);
        EXPECT_NE(item2, nullptr);
        EXPECT_EQ(item3, nullptr);

        delete item1;
        delete item2;
    }

    TEST(ObjectPoolTests, IncreaseSizePolicyGrowsCapacityPastLimit)
    {
        // See the note in ReturnNullPolicyReturnsNullPastCapacity above: needs 3 calls, not 2,
        // for Capacity(1) to actually be exceeded.
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 1, ObjectPoolIsFullPolicy::IncreaseSize);

        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();
        TestPoolable* item3 = pool.New();

        EXPECT_NE(item1, nullptr);
        EXPECT_NE(item2, nullptr);
        EXPECT_NE(item3, nullptr);
        EXPECT_EQ(pool.getCapacityProperty(), 2);
        EXPECT_EQ(pool.getTotalCountProperty(), 3);

        delete item1;
        delete item2;
        delete item3;
    }

    TEST(ObjectPoolTests, KillExistingPolicyReusesTheOldestInUseItem)
    {
        // See the note in ReturnNullPolicyReturnsNullPastCapacity above: needs 3 calls, not 2,
        // for Capacity(1) to actually be exceeded.
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 1, ObjectPoolIsFullPolicy::KillExisting);

        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();
        TestPoolable* item3 = pool.New(); // Capacity(1) exceeded by TotalCount(2) -- kills and reuses item1 (the head).

        EXPECT_EQ(item1, item3);
        EXPECT_NE(item2, item3);
        EXPECT_EQ(pool.getTotalCountProperty(), 2);

        delete item1;
        delete item2;
    }

    TEST(ObjectPoolTests, ItemUsedAndItemReturnedEventsFire)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); }, 2);

        std::vector<TestPoolable*> usedItems;
        std::vector<TestPoolable*> returnedItems;
        pool.ItemUsed.Add([&usedItems](TestPoolable* item) { usedItems.push_back(item); });
        pool.ItemReturned.Add([&returnedItems](TestPoolable* item) { returnedItems.push_back(item); });

        TestPoolable* item1 = pool.New();
        item1->Return();

        ASSERT_EQ(usedItems.size(), 1u);
        EXPECT_EQ(usedItems[0], item1);
        ASSERT_EQ(returnedItems.size(), 1u);
        EXPECT_EQ(returnedItems[0], item1);

        delete item1;
    }

    // Regression test for the severe upstream infinite-loop bug documented in ObjectPool.hpp's
    // header comment. Deliberately does NOT iterate the pool (see this file's header comment) --
    // that would hang the test process forever, since it IS the bug being demonstrated.
    TEST(ObjectPoolTests, FirstItemHasSelfReferencingLinksReproducesKnownUpstreamBug)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); });
        TestPoolable* item = pool.New();

        EXPECT_EQ(item->getNextNodeProperty(), static_cast<IPoolable*>(item))
            << "If this starts failing, upstream's ObjectPool self-referencing-node bug may have "
               "been fixed and this port's fidelity note should be revisited.";
        EXPECT_EQ(item->getPreviousNodeProperty(), static_cast<IPoolable*>(item));

        delete item;
    }

    // Regression test for the same bug: creating a second item retroactively fixes the FIRST
    // item's NextNode (via CreateObject's own bookkeeping), but never its PreviousNode -- which
    // remains self-referencing indefinitely, matching upstream exactly.
    TEST(ObjectPoolTests, SecondItemFixesFirstItemsNextNodeButNotPreviousNodeReproducesKnownUpstreamBug)
    {
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); });
        TestPoolable* item1 = pool.New();
        TestPoolable* item2 = pool.New();

        EXPECT_EQ(item1->getNextNodeProperty(), static_cast<IPoolable*>(item2));
        EXPECT_EQ(item1->getPreviousNodeProperty(), static_cast<IPoolable*>(item1))
            << "If this starts failing, upstream's ObjectPool self-referencing-node bug may have "
               "been fixed and this port's fidelity note should be revisited.";

        delete item1;
        delete item2;
    }

    TEST(ObjectPoolTests, ReturningTheSoleItemClearsItsTailSelfReference)
    {
        // Once an item is Return()ed, Return()'s own bookkeeping resets the (then-current) tail's
        // NextNode to null -- this is the specific mechanism that eventually breaks the
        // self-referencing loop, demonstrated directly here without ever iterating.
        ObjectPool<TestPoolable> pool([] { return new TestPoolable(); });
        TestPoolable* item = pool.New();
        item->Return();

        EXPECT_EQ(item->getNextNodeProperty(), nullptr);

        delete item;
    }
}
