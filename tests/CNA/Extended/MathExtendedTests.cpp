// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/MathExtendedTests.cs.
#include "CNA/Extended/MathExtended.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(MathExtendedTests, CalculateMinimumVector2ReturnsExpected)
    {
        const Vector2 a(1, 4);
        const Vector2 b(3, 2);

        const Vector2 expected(1, 2);
        const Vector2 actual = MathExtended::CalculateMinimumVector2(a, b);

        EXPECT_EQ(expected, actual);
    }

    TEST(MathExtendedTests, CalculateMinimumVector2OutExpected)
    {
        const Vector2 a(1, 4);
        const Vector2 b(3, 2);

        const Vector2 expected(1, 2);
        Vector2 actual;
        MathExtended::CalculateMinimumVector2(a, b, actual);

        EXPECT_EQ(expected, actual);
    }

    TEST(MathExtendedTests, CalculateMaximumVector2ReturnsExpected)
    {
        const Vector2 a(1, 4);
        const Vector2 b(3, 2);

        const Vector2 expected(3, 4);
        const Vector2 actual = MathExtended::CalculateMaximumVector2(a, b);

        EXPECT_EQ(expected, actual);
    }

    TEST(MathExtendedTests, CalculateMaximumVector2OutExpected)
    {
        const Vector2 a(1, 4);
        const Vector2 b(3, 2);

        const Vector2 expected(3, 4);
        Vector2 actual;
        MathExtended::CalculateMaximumVector2(a, b, actual);

        EXPECT_EQ(expected, actual);
    }
}
