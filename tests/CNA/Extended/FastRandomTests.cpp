// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for FastRandom.cs. These tests cover
// deterministic seeding, range bounds, and the Shared instance.
#include "CNA/Extended/FastRandom.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(FastRandomTests, SameSeedProducesSameSequence)
    {
        FastRandom a(42);
        FastRandom b(42);

        for (int i = 0; i < 10; i++)
        {
            EXPECT_EQ(a.Next(), b.Next());
        }
    }

    TEST(FastRandomTests, DifferentSeedsProduceDifferentFirstValue)
    {
        FastRandom a(1);
        FastRandom b(2);

        EXPECT_NE(a.Next(), b.Next());
    }

    TEST(FastRandomTests, NextMaxStaysWithinBounds)
    {
        FastRandom random(7);
        for (int i = 0; i < 200; i++)
        {
            const int value = random.Next(10);
            EXPECT_GE(value, 0);
            EXPECT_LE(value, 10);
        }
    }

    TEST(FastRandomTests, NextMinMaxStaysWithinBounds)
    {
        FastRandom random(7);
        for (int i = 0; i < 200; i++)
        {
            const int value = random.Next(5, 15);
            EXPECT_GE(value, 5);
            EXPECT_LE(value, 15);
        }
    }

    TEST(FastRandomTests, NextIntervalStaysWithinBounds)
    {
        FastRandom random(7);
        const Interval<int> interval(3, 8);
        for (int i = 0; i < 200; i++)
        {
            const int value = random.Next(interval);
            EXPECT_GE(value, 3);
            EXPECT_LE(value, 8);
        }
    }

    TEST(FastRandomTests, NextSingleStaysWithinZeroToOne)
    {
        FastRandom random(7);
        for (int i = 0; i < 200; i++)
        {
            const float value = random.NextSingle();
            EXPECT_GE(value, 0.0f);
            EXPECT_LT(value, 1.0f);
        }
    }

    TEST(FastRandomTests, NextSingleMinMaxStaysWithinBounds)
    {
        FastRandom random(7);
        for (int i = 0; i < 200; i++)
        {
            const float value = random.NextSingle(2.0f, 5.0f);
            EXPECT_GE(value, 2.0f);
            EXPECT_LT(value, 5.0f);
        }
    }

    TEST(FastRandomTests, NextAngleStaysWithinPiRange)
    {
        FastRandom random(7);
        for (int i = 0; i < 200; i++)
        {
            const float angle = random.NextAngle();
            EXPECT_GE(angle, -3.14159274f);
            EXPECT_LE(angle, 3.14159274f);
        }
    }

    TEST(FastRandomTests, NextUnitVectorProducesUnitLength)
    {
        FastRandom random(7);
        Vector2 vector;
        random.NextUnitVector(vector);
        EXPECT_NEAR(vector.Length(), 1.0f, 0.0001f);
    }

    TEST(FastRandomTests, NextUnitVectorPointerOverloadProducesUnitLength)
    {
        FastRandom random(7);
        Vector2 vector;
        random.NextUnitVector(&vector);
        EXPECT_NEAR(vector.Length(), 1.0f, 0.0001f);
    }

    TEST(FastRandomTests, SharedInstanceIsUsable)
    {
        FastRandom& shared = FastRandom::getSharedProperty();
        const int value = shared.Next(100);
        EXPECT_GE(value, 0);
        EXPECT_LE(value, 100);
    }

    TEST(FastRandomTests, SharedInstanceIsSingleton)
    {
        EXPECT_EQ(&FastRandom::getSharedProperty(), &FastRandom::getSharedProperty());
    }

    TEST(FastRandomTests, DefaultConstructorUsesSeedOne)
    {
        FastRandom defaultSeeded;
        FastRandom explicitlySeeded(1);
        EXPECT_EQ(defaultSeeded.Next(), explicitlySeeded.Next());
    }

    TEST(FastRandomTests, NonPositiveSeedThrows)
    {
        EXPECT_THROW(FastRandom{0}, std::out_of_range);
        EXPECT_THROW(FastRandom{-1}, std::out_of_range);
    }
}
