// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ParticleColorParameter/ParticleFloatParameter/
// ParticleInt32Parameter/ParticleVector2Parameter. Fresh tests below.
#include "CNA/Extended/Particles/Data/ParticleColorParameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleFloatParameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleInt32Parameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleVector2Parameter.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Data
{
    TEST(ParticleFloatParameterTests, ConstantValueReturnsConstant)
    {
        const ParticleFloatParameter subject(42.0f);
        EXPECT_FLOAT_EQ(subject.getValueProperty(), 42.0f);
    }

    TEST(ParticleFloatParameterTests, RandomValueLiesWithinRange)
    {
        const ParticleFloatParameter subject(1.0f, 5.0f);
        for (int i = 0; i < 20; ++i)
        {
            const float v = subject.getValueProperty();
            EXPECT_GE(v, 1.0f);
            EXPECT_LE(v, 5.0f);
        }
    }

    TEST(ParticleFloatParameterTests, EqualsComparesConstantValues)
    {
        EXPECT_EQ(ParticleFloatParameter(1.0f), ParticleFloatParameter(1.0f));
        EXPECT_NE(ParticleFloatParameter(1.0f), ParticleFloatParameter(2.0f));
    }

    TEST(ParticleInt32ParameterTests, ConstantValueReturnsConstant)
    {
        const ParticleInt32Parameter subject(7);
        EXPECT_EQ(subject.getValueProperty(), 7);
    }

    TEST(ParticleInt32ParameterTests, RandomValueLiesWithinRange)
    {
        const ParticleInt32Parameter subject(5, 10);
        for (int i = 0; i < 20; ++i)
        {
            const int v = subject.getValueProperty();
            EXPECT_GE(v, 5);
            EXPECT_LE(v, 10);
        }
    }

    TEST(ParticleVector2ParameterTests, ConstantValueReturnsConstant)
    {
        const ParticleVector2Parameter subject(Microsoft::Xna::Framework::Vector2(3.0f, 4.0f));
        const Microsoft::Xna::Framework::Vector2 value = subject.getValueProperty();
        EXPECT_FLOAT_EQ(value.X, 3.0f);
        EXPECT_FLOAT_EQ(value.Y, 4.0f);
    }

    TEST(ParticleVector2ParameterTests, UniformRandomSharesSameSampleForXAndY)
    {
        ParticleVector2Parameter subject(
            Microsoft::Xna::Framework::Vector2(1.0f, 1.0f), Microsoft::Xna::Framework::Vector2(2.0f, 2.0f));
        subject.Uniform = true;

        const Microsoft::Xna::Framework::Vector2 value = subject.getValueProperty();
        EXPECT_FLOAT_EQ(value.X, value.Y);
    }

    TEST(ParticleColorParameterTests, ConstantValueReturnsConstant)
    {
        const Microsoft::Xna::Framework::Vector3 hsl(0.5f, 0.6f, 0.7f);
        const ParticleColorParameter subject(hsl);
        const Microsoft::Xna::Framework::Vector3 value = subject.getValueProperty();
        EXPECT_FLOAT_EQ(value.X, 0.5f);
        EXPECT_FLOAT_EQ(value.Y, 0.6f);
        EXPECT_FLOAT_EQ(value.Z, 0.7f);
    }
}
