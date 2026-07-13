// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for GameComponentCollectionExtensions.cs. Fresh tests
// below.
#include "CNA/Extended/GameComponentCollectionExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    namespace
    {
        class TestGameComponent final : public IGameComponent
        {
        public:
            bool Initialized = false;

            void Initialize() override { Initialized = true; }
        };
    }

    TEST(GameComponentCollectionExtensionsTests, AddDefaultConstructsAndAddsComponent)
    {
        GameComponentCollection collection;
        TestGameComponent* component = Add<TestGameComponent>(collection);

        ASSERT_NE(component, nullptr);
        EXPECT_EQ(collection.getCountProperty(), 1u);
        EXPECT_TRUE(collection.Contains(component));

        delete component;
    }

    TEST(GameComponentCollectionExtensionsTests, AddWithFactoryUsesFactoryAndAddsComponent)
    {
        GameComponentCollection collection;
        auto* preConstructed = new TestGameComponent();
        preConstructed->Initialized = true;

        TestGameComponent* component = Add<TestGameComponent>(collection, [preConstructed]() { return preConstructed; });

        ASSERT_EQ(component, preConstructed);
        EXPECT_TRUE(component->Initialized);
        EXPECT_EQ(collection.getCountProperty(), 1u);
        EXPECT_TRUE(collection.Contains(component));

        delete component;
    }
}
