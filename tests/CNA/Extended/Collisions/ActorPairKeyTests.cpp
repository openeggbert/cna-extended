// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream ActorPairKeyTest.cs exists (it is exercised only indirectly, via
// CollisionWorld2D.QueryCollisionPairs's de-duplication in upstream's CollisionWorld2DTests.cs).
// Fresh tests below, covering the order-independence and hash-set-key usability this type exists
// for.
#include "CNA/Extended/Collisions/ActorPairKey.hpp"

#include "CNA/Extended/Collisions/ICollisionActor.hpp"

#include <gtest/gtest.h>
#include <unordered_set>

namespace CNA::Extended::Collisions
{
    namespace
    {
        class TestCollisionActor : public ICollisionActor
        {
        public:
            explicit TestCollisionActor(int id) : id_(id) {}

            [[nodiscard]] int getIdProperty() const override { return id_; }
            [[nodiscard]] CollisionShape2D getShapeProperty() const override { return CollisionShape2D(); }

        private:
            int id_;
        };
    }

    TEST(ActorPairKeyTests, OrdersByIdRegardlessOfConstructionOrder)
    {
        const TestCollisionActor lower(3);
        const TestCollisionActor higher(9);

        const ActorPairKey ascending(lower, higher);
        const ActorPairKey descending(higher, lower);

        EXPECT_EQ(ascending.FirstId, 3);
        EXPECT_EQ(ascending.SecondId, 9);
        EXPECT_EQ(descending.FirstId, 3);
        EXPECT_EQ(descending.SecondId, 9);
        EXPECT_EQ(ascending, descending);
    }

    TEST(ActorPairKeyTests, DifferentPairsAreNotEqual)
    {
        const TestCollisionActor a(1);
        const TestCollisionActor b(2);
        const TestCollisionActor c(3);

        const ActorPairKey ab(a, b);
        const ActorPairKey ac(a, c);
        EXPECT_NE(ab, ac);
    }

    TEST(ActorPairKeyTests, UsableAsUnorderedSetKeyAndDeduplicates)
    {
        const TestCollisionActor a(5);
        const TestCollisionActor b(7);

        std::unordered_set<ActorPairKey> seen;
        EXPECT_TRUE(seen.insert(ActorPairKey(a, b)).second);
        EXPECT_FALSE(seen.insert(ActorPairKey(b, a)).second);
        EXPECT_EQ(seen.size(), 1u);
    }
}
