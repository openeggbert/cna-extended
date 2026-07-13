// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream LayerPairTest.cs exists (LayerPair is `internal` and only exercised indirectly,
// through CollisionWorld2D's own test suite, not yet ported). Fresh tests below.
#include "CNA/Extended/Collisions/LayerPair.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"
#include "CNA/Extended/Collisions/Layer.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

namespace CNA::Extended::Collisions
{
    namespace
    {
        class FakeBroadphase : public ICollisionBroadphase2D
        {
        public:
            void Insert(ICollisionActor*) override {}
            bool Remove(ICollisionActor*) override { return false; }
            [[nodiscard]] std::vector<ICollisionActor*> Query(const BoundingBox2D&) const override { return {}; }
            [[nodiscard]] const std::vector<ICollisionActor*>& GetActors() const override { return actors_; }
            void Reset() override {}

        private:
            std::vector<ICollisionActor*> actors_;
        };

        std::unique_ptr<Layer> MakeLayer() { return std::make_unique<Layer>(std::make_unique<FakeBroadphase>()); }
    }

    TEST(LayerPairTests, EqualRegardlessOfConstructionOrder)
    {
        const std::unique_ptr<Layer> a = MakeLayer();
        const std::unique_ptr<Layer> b = MakeLayer();

        const LayerPair ab(*a, *b);
        const LayerPair ba(*b, *a);
        EXPECT_EQ(ab, ba);
    }

    TEST(LayerPairTests, SelfPairIsEqualToItself)
    {
        const std::unique_ptr<Layer> a = MakeLayer();
        const LayerPair aa(*a, *a);
        EXPECT_EQ(aa.First, aa.Second);
    }

    TEST(LayerPairTests, DifferentPairsAreNotEqual)
    {
        const std::unique_ptr<Layer> a = MakeLayer();
        const std::unique_ptr<Layer> b = MakeLayer();
        const std::unique_ptr<Layer> c = MakeLayer();

        const LayerPair ab(*a, *b);
        const LayerPair ac(*a, *c);
        EXPECT_NE(ab, ac);
    }

    TEST(LayerPairTests, UsableAsUnorderedSetKeyAndDeduplicates)
    {
        const std::unique_ptr<Layer> a = MakeLayer();
        const std::unique_ptr<Layer> b = MakeLayer();

        std::unordered_set<LayerPair> seen;
        EXPECT_TRUE(seen.insert(LayerPair(*a, *b)).second);
        EXPECT_FALSE(seen.insert(LayerPair(*b, *a)).second);
        EXPECT_EQ(seen.size(), 1u);
    }
}
