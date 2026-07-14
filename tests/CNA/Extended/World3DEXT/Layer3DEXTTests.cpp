// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Layer3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here. Mirrors CNA::Extended::Collisions::LayerTests.cpp
// (2026-07-14, user-requested parity work), using a minimal ICollisionBroadphase3DEXT fake
// that just tracks whether Reset() was called.
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ICollisionActor3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ICollisionBroadphase3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>

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
            void Reset() override { resetCallCount++; }

            int resetCallCount = 0;

        private:
            std::vector<ICollisionActor3DEXT*> actors_;
        };
    }

    TEST(Layer3DEXTTests, ConstructorThrowsOnNullSpace)
    {
        EXPECT_THROW(Layer3DEXT layer(nullptr), std::invalid_argument);
    }

    TEST(Layer3DEXTTests, IsDynamicDefaultsToTrue)
    {
        const Layer3DEXT layer(std::make_unique<FakeBroadphase3DEXT>());
        EXPECT_TRUE(layer.getIsDynamicProperty());
    }

    TEST(Layer3DEXTTests, ResetCallsSpaceResetWhenDynamic)
    {
        auto broadphase = std::make_unique<FakeBroadphase3DEXT>();
        FakeBroadphase3DEXT* broadphasePtr = broadphase.get();
        Layer3DEXT layer(std::move(broadphase));

        layer.Reset();
        EXPECT_EQ(broadphasePtr->resetCallCount, 1);
    }

    TEST(Layer3DEXTTests, ResetDoesNotCallSpaceResetWhenNotDynamic)
    {
        auto broadphase = std::make_unique<FakeBroadphase3DEXT>();
        FakeBroadphase3DEXT* broadphasePtr = broadphase.get();
        Layer3DEXT layer(std::move(broadphase));

        layer.setIsDynamicProperty(false);
        layer.Reset();
        EXPECT_EQ(broadphasePtr->resetCallCount, 0);
    }

    TEST(Layer3DEXTTests, GetSpacePropertyReturnsTheOwnedBroadphase)
    {
        auto broadphase = std::make_unique<FakeBroadphase3DEXT>();
        FakeBroadphase3DEXT* broadphasePtr = broadphase.get();
        const Layer3DEXT layer(std::move(broadphase));

        EXPECT_EQ(&layer.getSpaceProperty(), broadphasePtr);
    }
}
