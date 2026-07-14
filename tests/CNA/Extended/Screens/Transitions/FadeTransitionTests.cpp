// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for FadeTransition. Fresh tests below. A plain
// `GraphicsDevice`/`SpriteBatch` pair genuinely constructs and renders end-to-end headlessly
// in this environment (real EasyGL-over-Mesa software rendering, confirmed this session) --
// so `Draw` here is exercised with a real internal `SpriteBatch`, not just compile-checked.
#include "CNA/Extended/Screens/Transitions/FadeTransition.hpp"

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "System/TimeSpan.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Screens::Transitions
{
    using Microsoft::Xna::Framework::GameTime;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Viewport;
    using System::TimeSpan;

    namespace
    {
        class FadeTransitionTest : public ::testing::Test
        {
        protected:
            void SetUp() override { graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480)); }

            GraphicsDevice graphicsDevice;
        };
    }

    TEST_F(FadeTransitionTest, Draw_AtStart_DoesNotThrow)
    {
        FadeTransition transition(graphicsDevice, Color::Black);
        GameTime gameTime;
        EXPECT_NO_THROW(transition.Draw(gameTime));
    }

    TEST_F(FadeTransitionTest, Draw_MidTransition_DoesNotThrow)
    {
        FadeTransition transition(graphicsDevice, Color::Black);
        GameTime updateTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.25));
        transition.Update(updateTime);

        EXPECT_GT(transition.getValueProperty(), 0.0f);

        GameTime drawTime;
        EXPECT_NO_THROW(transition.Draw(drawTime));
    }

    TEST_F(FadeTransitionTest, Draw_AfterFullOutHalf_DoesNotThrow)
    {
        FadeTransition transition(graphicsDevice, Color::Red, 1.0f);
        GameTime updateTime(TimeSpan::Zero, TimeSpan::FromSeconds(0.6));
        transition.Update(updateTime);

        EXPECT_EQ(transition.getStateProperty(), TransitionState::In);

        GameTime drawTime;
        EXPECT_NO_THROW(transition.Draw(drawTime));
    }

    TEST_F(FadeTransitionTest, GetColorProperty_ReturnsConstructedColor)
    {
        const FadeTransition transition(graphicsDevice, Color::Blue);
        EXPECT_EQ(transition.getColorProperty(), Color::Blue);
    }
}
