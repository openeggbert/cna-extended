// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for LayerPair3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here. Mirrors
// CNA::Extended::Collisions::LayerPairTests.cpp (2026-07-14, user-requested parity work).
#include "CNA/Extended/World3DEXT/LayerPair3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

namespace CNA::Extended::World3DEXT
{
    namespace
    {
        class FakeBroadphase3DEXT : public ICollisionBroadphase3DEXT
        {
        public:
            void Insert(ICollisionActor3DEXT*) override {}
            bool Remove(ICollisionActor3DEXT*) override { return false; }
            [[nodiscard]] std::vector<ICollisionActor3DEXT*> Query(const Microsoft::Xna::Framework::BoundingBox&) const override { return {}; }
            [[nodiscard]] const std::vector<ICollisionActor3DEXT*>& GetActors() const override { return actors_; }
            void Reset() override {}

        private:
            std::vector<ICollisionActor3DEXT*> actors_;
        };

        std::unique_ptr<Layer3DEXT> MakeLayer() { return std::make_unique<Layer3DEXT>(std::make_unique<FakeBroadphase3DEXT>()); }
    }

    TEST(LayerPair3DEXTTests, EqualRegardlessOfConstructionOrder)
    {
        const std::unique_ptr<Layer3DEXT> a = MakeLayer();
        const std::unique_ptr<Layer3DEXT> b = MakeLayer();

        const LayerPair3DEXT ab(*a, *b);
        const LayerPair3DEXT ba(*b, *a);
        EXPECT_EQ(ab, ba);
    }

    TEST(LayerPair3DEXTTests, SelfPairIsEqualToItself)
    {
        const std::unique_ptr<Layer3DEXT> a = MakeLayer();
        const LayerPair3DEXT aa(*a, *a);
        EXPECT_EQ(aa.First, aa.Second);
    }

    TEST(LayerPair3DEXTTests, DifferentPairsAreNotEqual)
    {
        const std::unique_ptr<Layer3DEXT> a = MakeLayer();
        const std::unique_ptr<Layer3DEXT> b = MakeLayer();
        const std::unique_ptr<Layer3DEXT> c = MakeLayer();

        const LayerPair3DEXT ab(*a, *b);
        const LayerPair3DEXT ac(*a, *c);
        EXPECT_NE(ab, ac);
    }

    TEST(LayerPair3DEXTTests, UsableAsUnorderedSetKeyAndDeduplicates)
    {
        const std::unique_ptr<Layer3DEXT> a = MakeLayer();
        const std::unique_ptr<Layer3DEXT> b = MakeLayer();

        std::unordered_set<LayerPair3DEXT> seen;
        EXPECT_TRUE(seen.insert(LayerPair3DEXT(*a, *b)).second);
        EXPECT_FALSE(seen.insert(LayerPair3DEXT(*b, *a)).second);
        EXPECT_EQ(seen.size(), 1u);
    }
}
