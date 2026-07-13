// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/ComponentMapperTests.cs.
// `mapper.Put(entityId, new Transform2())`-style upstream calls allocate a heap component and
// never free it (matching this module's caller-owns-components ownership model -- see
// ComponentMapper.hpp's header comment); ported using stack-local `Transform2` instances instead,
// which produce identical observable test behavior (Put/Get/Has/Delete all operate on the pointer
// identity, not on how the pointee was allocated) without leaking.
#include "CNA/Extended/ECS/ComponentMapper.hpp"

#include "CNA/Extended/Transform.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::ECS
{
    TEST(ComponentMapperTests, CreateComponentMapper)
    {
        ComponentMapperOf<int> mapper(0, nullptr);

        EXPECT_EQ(mapper.getComponentTypeProperty(), std::type_index(typeid(int)));
        EXPECT_EQ(mapper.getComponentsProperty().getCountProperty(), 0u);
    }

    TEST(ComponentMapperTests, OnPut)
    {
        constexpr int entityId = 3;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(1, &noopChanged);
        Transform2 component;

        bool called = false;
        mapper.OnPut += [&](int entId)
        {
            EXPECT_EQ(entId, entityId);
            EXPECT_EQ(mapper.Get(entityId), &component);
            called = true;
        };

        mapper.Put(entityId, &component);
        EXPECT_TRUE(called);
    }

    TEST(ComponentMapperTests, PutAndGetComponent)
    {
        constexpr int entityId = 3;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(1, &noopChanged);
        Transform2 component;

        mapper.Put(entityId, &component);

        EXPECT_EQ(mapper.getComponentTypeProperty(), std::type_index(typeid(Transform2)));
        EXPECT_GE(mapper.getComponentsProperty().getCountProperty(), 1u);
        EXPECT_EQ(mapper.Get(entityId), &component);
    }

    TEST(ComponentMapperTests, PutAndTryGetComponent)
    {
        constexpr int entityId = 4;
        constexpr int entityIdNotAdded = 100;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(1, &noopChanged);
        Transform2 component;

        mapper.Put(entityId, &component);

        Transform2* fromAdded = nullptr;
        Transform2* fromNotAdded = nullptr;
        const bool foundAdded = mapper.TryGet(entityId, fromAdded);
        const bool foundNotAdded = mapper.TryGet(entityIdNotAdded, fromNotAdded);

        EXPECT_EQ(mapper.getComponentTypeProperty(), std::type_index(typeid(Transform2)));
        EXPECT_GE(mapper.getComponentsProperty().getCountProperty(), 1u);
        EXPECT_TRUE(foundAdded);
        EXPECT_EQ(fromAdded, &component);
        EXPECT_FALSE(foundNotAdded);
        EXPECT_EQ(fromNotAdded, nullptr);
    }

    TEST(ComponentMapperTests, OnDelete)
    {
        constexpr int entityId = 1;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(2, &noopChanged);
        Transform2 component;

        bool called = false;
        mapper.OnDelete += [&](int entId)
        {
            EXPECT_EQ(entId, entityId);
            EXPECT_TRUE(mapper.Has(entityId));
            EXPECT_NE(mapper.Get(entityId), nullptr);
            called = true;
        };

        mapper.Put(entityId, &component);
        mapper.Delete(entityId);

        EXPECT_TRUE(called);
        EXPECT_FALSE(mapper.Has(entityId));
    }

    TEST(ComponentMapperTests, DeleteComponent)
    {
        constexpr int entityId = 1;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(2, &noopChanged);
        Transform2 component;

        mapper.Put(entityId, &component);
        mapper.Delete(entityId);

        EXPECT_FALSE(mapper.Has(entityId));
    }

    TEST(ComponentMapperTests, HasComponent)
    {
        constexpr int entityId = 0;
        System::MulticastAction<int> noopChanged;

        ComponentMapperOf<Transform2> mapper(3, &noopChanged);
        Transform2 component;

        EXPECT_FALSE(mapper.Has(entityId));

        mapper.Put(entityId, &component);

        EXPECT_TRUE(mapper.Has(entityId));
    }
}
