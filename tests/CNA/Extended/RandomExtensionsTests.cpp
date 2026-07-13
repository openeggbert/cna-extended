// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for RandomExtensions.cs. These tests cover
// range bounds for each free function.
#include "CNA/Extended/RandomExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(RandomExtensionsTests, NextIntervalStaysWithinBounds)
    {
        System::Random random(7);
        const Interval<int> interval(3, 8);
        for (int i = 0; i < 200; i++)
        {
            const int value = Next(random, interval);
            EXPECT_GE(value, 3);
            EXPECT_LE(value, 8);
        }
    }

    TEST(RandomExtensionsTests, NextSingleStaysWithinZeroToOne)
    {
        System::Random random(7);
        for (int i = 0; i < 200; i++)
        {
            const float value = NextSingle(random);
            EXPECT_GE(value, 0.0f);
            EXPECT_LT(value, 1.0f);
        }
    }

    TEST(RandomExtensionsTests, NextSingleMaxStaysWithinBounds)
    {
        System::Random random(7);
        for (int i = 0; i < 200; i++)
        {
            const float value = NextSingle(random, 5.0f);
            EXPECT_GE(value, 0.0f);
            EXPECT_LT(value, 5.0f);
        }
    }

    TEST(RandomExtensionsTests, NextSingleMinMaxStaysWithinBounds)
    {
        System::Random random(7);
        for (int i = 0; i < 200; i++)
        {
            const float value = NextSingle(random, 2.0f, 5.0f);
            EXPECT_GE(value, 2.0f);
            EXPECT_LT(value, 5.0f);
        }
    }

    TEST(RandomExtensionsTests, NextSingleIntervalStaysWithinBounds)
    {
        System::Random random(7);
        const Interval<float> interval(2.0f, 5.0f);
        for (int i = 0; i < 200; i++)
        {
            const float value = NextSingle(random, interval);
            EXPECT_GE(value, 2.0f);
            EXPECT_LT(value, 5.0f);
        }
    }

    TEST(RandomExtensionsTests, NextAngleStaysWithinPiRange)
    {
        System::Random random(7);
        for (int i = 0; i < 200; i++)
        {
            const float angle = NextAngle(random);
            EXPECT_GE(angle, -3.14159274f);
            EXPECT_LE(angle, 3.14159274f);
        }
    }

    TEST(RandomExtensionsTests, NextUnitVectorProducesUnitLength)
    {
        System::Random random(7);
        Vector2 vector;
        NextUnitVector(random, vector);
        EXPECT_NEAR(vector.Length(), 1.0f, 0.0001f);
    }
}
