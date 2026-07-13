// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/ECS/AspectTests.cs.
#include "CNA/Extended/ECS/Aspect.hpp"

#include "CNA/Extended/ECS/AspectBuilder.hpp"
#include "CNA/Extended/ECS/ComponentManager.hpp"
#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Transform.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::ECS
{
    namespace
    {
        class DummyComponent
        {
        };

        class AspectTest : public ::testing::Test
        {
        protected:
            void SetUp() override
            {
                // EntityA has Transform2, Sprite, and DummyComponent
                entityA_[componentManager_.GetComponentTypeId(std::type_index(typeid(Transform2)))] = true;
                entityA_[componentManager_.GetComponentTypeId(std::type_index(typeid(Graphics::Sprite)))] = true;
                entityA_[componentManager_.GetComponentTypeId(std::type_index(typeid(DummyComponent)))] = true;

                // EntityB has Transform2 and Sprite only
                entityB_[componentManager_.GetComponentTypeId(std::type_index(typeid(Transform2)))] = true;
                entityB_[componentManager_.GetComponentTypeId(std::type_index(typeid(Graphics::Sprite)))] = true;
            }

            ComponentManager componentManager_;
            ComponentBits entityA_;
            ComponentBits entityB_;
        };
    }

    TEST_F(AspectTest, EmptyAspectMatchesAllComponents)
    {
        ComponentManager componentManager;
        const Aspect emptyAspect = Aspect::All().Build(componentManager);

        EXPECT_TRUE(emptyAspect.IsInterested(entityA_));
        EXPECT_TRUE(emptyAspect.IsInterested(entityB_));
    }

    TEST_F(AspectTest, IsInterestedInAllComponents)
    {
        const Aspect allAspect =
            Aspect::All({std::type_index(typeid(Graphics::Sprite)), std::type_index(typeid(Transform2)), std::type_index(typeid(DummyComponent))})
                .Build(componentManager_);

        EXPECT_TRUE(allAspect.IsInterested(entityA_));
        EXPECT_FALSE(allAspect.IsInterested(entityB_));
    }

    TEST_F(AspectTest, IsInterestedInEitherOneOfTheComponents)
    {
        const Aspect eitherOneAspect =
            Aspect::One({std::type_index(typeid(Transform2)), std::type_index(typeid(DummyComponent))}).Build(componentManager_);

        EXPECT_TRUE(eitherOneAspect.IsInterested(entityA_));
        EXPECT_TRUE(eitherOneAspect.IsInterested(entityB_));
    }

    TEST_F(AspectTest, IsInterestedInJustOneComponent)
    {
        const Aspect oneAspect = Aspect::One({std::type_index(typeid(DummyComponent))}).Build(componentManager_);

        EXPECT_TRUE(oneAspect.IsInterested(entityA_));
        EXPECT_FALSE(oneAspect.IsInterested(entityB_));
    }

    TEST_F(AspectTest, IsInterestedInExcludingOneComponent)
    {
        const Aspect oneAspect = Aspect::Exclude({std::type_index(typeid(DummyComponent))}).Build(componentManager_);

        EXPECT_FALSE(oneAspect.IsInterested(entityA_));
        EXPECT_TRUE(oneAspect.IsInterested(entityB_));
    }
}
