// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no upstream tests for SimpleGameComponent.cs or
// SimpleDrawableGameComponent.cs. Fresh tests below, using minimal concrete subclasses since both
// are abstract.
#include "CNA/Extended/SimpleDrawableGameComponent.hpp"
#include "CNA/Extended/SimpleGameComponent.hpp"

#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using System::TimeSpan;

    namespace
    {
        class TestSimpleGameComponent final : public SimpleGameComponent
        {
        public:
            int UpdateCount = 0;
            int LoadContentCount = 0;
            int UnloadContentCount = 0;

            void Update(GameTime&) override { UpdateCount++; }

            [[nodiscard]] const std::string& GetTypeName() const override
            {
                static const std::string typeName = "CNA.Extended.Tests.TestSimpleGameComponent";
                return typeName;
            }

        protected:
            void LoadContent() override { LoadContentCount++; }
            void UnloadContent() override { UnloadContentCount++; }
        };

        class TestSimpleDrawableGameComponent final : public SimpleDrawableGameComponent
        {
        public:
            int DrawCount = 0;

            void Update(GameTime&) override
            {
            }

            void Draw(const GameTime&) override { DrawCount++; }

            [[nodiscard]] const std::string& GetTypeName() const override
            {
                static const std::string typeName = "CNA.Extended.Tests.TestSimpleDrawableGameComponent";
                return typeName;
            }
        };
    }

    TEST(SimpleGameComponentTests, IsEnabledDefaultsToTrue)
    {
        const TestSimpleGameComponent component;
        EXPECT_TRUE(component.getIsEnabledProperty());
    }

    TEST(SimpleGameComponentTests, IsEnabledSetterRaisesEnabledChangedOnlyWhenValueChanges)
    {
        TestSimpleGameComponent component;
        int raiseCount = 0;
        component.EnabledChanged.Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        component.setIsEnabledProperty(false);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_FALSE(component.getIsEnabledProperty());

        component.setIsEnabledProperty(false);
        EXPECT_EQ(raiseCount, 1);
    }

    TEST(SimpleGameComponentTests, InterfaceEnabledMatchesPublicIsEnabled)
    {
        TestSimpleGameComponent component;
        component.setIsEnabledProperty(false);

        const IUpdateable& asUpdateable = component;
        EXPECT_EQ(asUpdateable.getEnabledProperty(), component.getIsEnabledProperty());
    }

    TEST(SimpleGameComponentTests, UpdateOrderSetterRaisesUpdateOrderChangedOnlyWhenValueChanges)
    {
        TestSimpleGameComponent component;
        int raiseCount = 0;
        component.UpdateOrderChanged.Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        component.setUpdateOrderProperty(3);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_EQ(component.getUpdateOrderProperty(), 3);

        component.setUpdateOrderProperty(3);
        EXPECT_EQ(raiseCount, 1);
    }

    TEST(SimpleGameComponentTests, InitializeCallsLoadContentOnlyOnce)
    {
        TestSimpleGameComponent component;
        component.Initialize();
        component.Initialize();

        EXPECT_EQ(component.LoadContentCount, 1);
    }

    TEST(SimpleGameComponentTests, DisposeCallsUnloadContentOnlyIfInitialized)
    {
        TestSimpleGameComponent component;
        component.Dispose();
        EXPECT_EQ(component.UnloadContentCount, 0);

        component.Initialize();
        component.Dispose();
        EXPECT_EQ(component.UnloadContentCount, 1);

        component.Dispose();
        EXPECT_EQ(component.UnloadContentCount, 1);
    }

    TEST(SimpleGameComponentTests, UpdateDelegatesToOverride)
    {
        TestSimpleGameComponent component;
        GameTime gameTime(TimeSpan::FromSeconds(0.0), TimeSpan::FromSeconds(0.1));
        component.Update(gameTime);
        EXPECT_EQ(component.UpdateCount, 1);
    }

    TEST(SimpleGameComponentTests, CompareToOrdersByUpdateOrderDescending)
    {
        TestSimpleGameComponent first;
        TestSimpleGameComponent second;
        first.setUpdateOrderProperty(1);
        second.setUpdateOrderProperty(5);

        EXPECT_EQ(first.CompareTo(second), 4);
        EXPECT_EQ(second.CompareTo(first), -4);
    }

    TEST(SimpleDrawableGameComponentTests, VisibleDefaultsToTrue)
    {
        const TestSimpleDrawableGameComponent component;
        EXPECT_TRUE(component.getVisibleProperty());
    }

    TEST(SimpleDrawableGameComponentTests, VisibleSetterRaisesVisibleChangedOnlyWhenValueChanges)
    {
        TestSimpleDrawableGameComponent component;
        int raiseCount = 0;
        component.VisibleChanged.Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        component.setVisibleProperty(false);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_FALSE(component.getVisibleProperty());

        component.setVisibleProperty(false);
        EXPECT_EQ(raiseCount, 1);
    }

    TEST(SimpleDrawableGameComponentTests, DrawOrderSetterRaisesDrawOrderChangedOnlyWhenValueChanges)
    {
        TestSimpleDrawableGameComponent component;
        int raiseCount = 0;
        component.DrawOrderChanged.Add([&raiseCount](System::Object*, const System::EventArgs&) { raiseCount++; });

        component.setDrawOrderProperty(2);
        EXPECT_EQ(raiseCount, 1);
        EXPECT_EQ(component.getDrawOrderProperty(), 2);

        component.setDrawOrderProperty(2);
        EXPECT_EQ(raiseCount, 1);
    }

    TEST(SimpleDrawableGameComponentTests, DrawDelegatesToOverride)
    {
        TestSimpleDrawableGameComponent component;
        GameTime gameTime(TimeSpan::FromSeconds(0.0), TimeSpan::FromSeconds(0.1));
        component.Draw(gameTime);
        EXPECT_EQ(component.DrawCount, 1);
    }

    TEST(SimpleDrawableGameComponentTests, IsAlsoASimpleGameComponent)
    {
        TestSimpleDrawableGameComponent component;
        component.setIsEnabledProperty(false);
        EXPECT_FALSE(component.getIsEnabledProperty());
    }
}
