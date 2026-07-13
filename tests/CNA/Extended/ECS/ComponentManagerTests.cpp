// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/ComponentManagerTests.cs.
// `GetMapperForTypeByIndexer` is adapted -- see ComponentManager.hpp's header comment for why
// `GetMapper(std::type_index)` cannot construct a mapper for a type it has never seen via the
// generic `GetMapper<T>()` path the way upstream's reflection-based indexer can. The adapted test
// establishes the mappers via `GetMapper<T>()` first, then verifies type_index-based lookup
// returns the same instances -- the part of upstream's behavior that IS faithfully reproducible.
// `EnumerateMappers` substitutes a local `ThirdComponent` for upstream's `OrthographicCamera`
// (not yet ported in this project) -- only type-distinctness matters for this test, not the
// specific type used.
#include "CNA/Extended/ECS/ComponentManager.hpp"

#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Transform.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::ECS
{
    namespace
    {
        class ThirdComponent
        {
        };
    }

    TEST(ComponentManagerTests, GetMapperForType)
    {
        ComponentManager componentManager;
        ComponentMapperOf<Transform2>* transformMapper = componentManager.GetMapper<Transform2>();
        ComponentMapperOf<Graphics::Sprite>* spriteMapper = componentManager.GetMapper<Graphics::Sprite>();

        EXPECT_EQ(transformMapper->getIdProperty(), 0);
        EXPECT_EQ(spriteMapper->getIdProperty(), 1);
        EXPECT_EQ(spriteMapper, componentManager.GetMapper<Graphics::Sprite>());
    }

    TEST(ComponentManagerTests, GetComponentTypeId)
    {
        ComponentManager componentManager;

        EXPECT_EQ(componentManager.GetComponentTypeId(std::type_index(typeid(Transform2))), 0);
        EXPECT_EQ(componentManager.GetComponentTypeId(std::type_index(typeid(Graphics::Sprite))), 1);
        EXPECT_EQ(componentManager.GetComponentTypeId(std::type_index(typeid(Transform2))), 0);
    }

    TEST(ComponentManagerTests, GetMapperForTypeByIndexer)
    {
        ComponentManager componentManager;
        ComponentMapperOf<Transform2>* transformMapper = componentManager.GetMapper<Transform2>();
        ComponentMapperOf<Graphics::Sprite>* spriteMapper = componentManager.GetMapper<Graphics::Sprite>();

        ComponentMapper* transformMapperByIndexer = componentManager.GetMapper(std::type_index(typeid(Transform2)));
        ComponentMapper* spriteMapperByIndexer = componentManager.GetMapper(std::type_index(typeid(Graphics::Sprite)));

        EXPECT_EQ(transformMapperByIndexer, transformMapper);
        EXPECT_EQ(spriteMapperByIndexer, spriteMapper);
        EXPECT_EQ(transformMapperByIndexer->getIdProperty(), 0);
        EXPECT_EQ(spriteMapperByIndexer->getIdProperty(), 1);
        EXPECT_EQ(spriteMapperByIndexer, componentManager.GetMapper(std::type_index(typeid(Graphics::Sprite))));
    }

    TEST(ComponentManagerTests, EnumerateMappers)
    {
        ComponentManager componentManager;

        ComponentMapperOf<Transform2>* transformMapper = componentManager.GetMapper<Transform2>();
        ComponentMapperOf<Graphics::Sprite>* spriteMapper = componentManager.GetMapper<Graphics::Sprite>();
        ComponentMapperOf<ThirdComponent>* thirdMapper = componentManager.GetMapper<ThirdComponent>();

        std::vector<ComponentMapper*> enumeratedMappers;
        for (ComponentMapper* mapper : componentManager)
        {
            enumeratedMappers.push_back(mapper);
        }

        ASSERT_EQ(enumeratedMappers.size(), 3u);
        EXPECT_NE(std::find(enumeratedMappers.begin(), enumeratedMappers.end(), transformMapper), enumeratedMappers.end());
        EXPECT_NE(std::find(enumeratedMappers.begin(), enumeratedMappers.end(), spriteMapper), enumeratedMappers.end());
        EXPECT_NE(std::find(enumeratedMappers.begin(), enumeratedMappers.end(), thirdMapper), enumeratedMappers.end());
    }
}
