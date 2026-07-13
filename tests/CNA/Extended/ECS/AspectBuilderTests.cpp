// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/AspectBuilderTests.cs.
// Upstream's `typeof(string)`/`typeof(Texture2D)` placeholders (used only for type identity, never
// instantiated) are ported as `typeid(std::string)`/`typeid(Texture2D)`.
#include "CNA/Extended/ECS/AspectBuilder.hpp"

#include "CNA/Extended/ECS/Aspect.hpp"
#include "CNA/Extended/ECS/ComponentManager.hpp"
#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Transform.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <string>

namespace CNA::Extended::ECS
{
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    namespace
    {
        bool Contains(const std::vector<std::type_index>& types, std::type_index type)
        {
            return std::find(types.begin(), types.end(), type) != types.end();
        }
    }

    TEST(AspectBuilderTests, MatchAllTypes)
    {
        AspectBuilder builder;
        builder.All({std::type_index(typeid(Transform2)), std::type_index(typeid(Graphics::Sprite))});

        EXPECT_EQ(builder.getAllTypesProperty().size(), 2u);
        EXPECT_TRUE(Contains(builder.getAllTypesProperty(), std::type_index(typeid(Transform2))));
        EXPECT_TRUE(Contains(builder.getAllTypesProperty(), std::type_index(typeid(Graphics::Sprite))));
    }

    TEST(AspectBuilderTests, MatchAllTypesIsEmpty)
    {
        AspectBuilder builder;
        builder.All();

        EXPECT_TRUE(builder.getAllTypesProperty().empty());
        EXPECT_TRUE(builder.getOneTypesProperty().empty());
        EXPECT_TRUE(builder.getExclusionTypesProperty().empty());
    }

    TEST(AspectBuilderTests, MatchOneOfType)
    {
        AspectBuilder builder;
        builder.One({std::type_index(typeid(Transform2)), std::type_index(typeid(Graphics::Sprite))});

        EXPECT_EQ(builder.getOneTypesProperty().size(), 2u);
        EXPECT_TRUE(Contains(builder.getOneTypesProperty(), std::type_index(typeid(Transform2))));
        EXPECT_TRUE(Contains(builder.getOneTypesProperty(), std::type_index(typeid(Graphics::Sprite))));
    }

    TEST(AspectBuilderTests, ExcludeTypes)
    {
        AspectBuilder builder;
        builder.Exclude({std::type_index(typeid(Transform2)), std::type_index(typeid(Graphics::Sprite))});

        EXPECT_EQ(builder.getExclusionTypesProperty().size(), 2u);
        EXPECT_TRUE(Contains(builder.getExclusionTypesProperty(), std::type_index(typeid(Transform2))));
        EXPECT_TRUE(Contains(builder.getExclusionTypesProperty(), std::type_index(typeid(Graphics::Sprite))));
    }

    TEST(AspectBuilderTests, BuildAspect)
    {
        ComponentManager componentManager;
        AspectBuilder builder;
        builder.All({std::type_index(typeid(Transform2)), std::type_index(typeid(Graphics::Sprite))})
            .One({std::type_index(typeid(std::string))})
            .Exclude({std::type_index(typeid(Texture2D))});

        const Aspect aspect = builder.Build(componentManager);

        EXPECT_FALSE(aspect.AllSet.getIsEmptyProperty());
        EXPECT_FALSE(aspect.OneSet.getIsEmptyProperty());
        EXPECT_FALSE(aspect.ExclusionSet.getIsEmptyProperty());
    }
}
