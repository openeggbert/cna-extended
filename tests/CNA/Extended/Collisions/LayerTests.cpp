// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream LayerTest.cs exists (Layer is only exercised indirectly, through CollisionWorld2D's
// own test suite, not yet ported). Fresh tests below, using a minimal ICollisionBroadphase2D fake
// that just tracks whether Reset() was called.
#include "CNA/Extended/Collisions/Layer.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/Collisions/ICollisionActor.hpp"
#include "CNA/Extended/Collisions/ICollisionBroadphase2D.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

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
            void Reset() override { resetCallCount++; }

            int resetCallCount = 0;

        private:
            std::vector<ICollisionActor*> actors_;
        };
    }

    TEST(LayerTests, ConstructorThrowsOnNullSpace)
    {
        EXPECT_THROW(Layer layer(nullptr), std::invalid_argument);
    }

    TEST(LayerTests, IsDynamicDefaultsToTrue)
    {
        const Layer layer(std::make_unique<FakeBroadphase>());
        EXPECT_TRUE(layer.getIsDynamicProperty());
    }

    TEST(LayerTests, ResetCallsSpaceResetWhenDynamic)
    {
        auto broadphase = std::make_unique<FakeBroadphase>();
        FakeBroadphase* broadphasePtr = broadphase.get();
        Layer layer(std::move(broadphase));

        layer.Reset();
        EXPECT_EQ(broadphasePtr->resetCallCount, 1);
    }

    TEST(LayerTests, ResetDoesNotCallSpaceResetWhenNotDynamic)
    {
        auto broadphase = std::make_unique<FakeBroadphase>();
        FakeBroadphase* broadphasePtr = broadphase.get();
        Layer layer(std::move(broadphase));

        layer.setIsDynamicProperty(false);
        layer.Reset();
        EXPECT_EQ(broadphasePtr->resetCallCount, 0);
    }

    TEST(LayerTests, GetSpacePropertyReturnsTheOwnedBroadphase)
    {
        auto broadphase = std::make_unique<FakeBroadphase>();
        FakeBroadphase* broadphasePtr = broadphase.get();
        const Layer layer(std::move(broadphase));

        EXPECT_EQ(&layer.getSpaceProperty(), broadphasePtr);
    }
}
