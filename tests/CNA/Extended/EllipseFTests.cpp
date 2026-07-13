// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Primitives/EllipseFTest.cs.
#include "CNA/Extended/EllipseF.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    struct EllipseFContainsCase
    {
        float x;
        float y;
        bool expected;
    };

    class EllipseFContainsPointCircleTests : public ::testing::TestWithParam<EllipseFContainsCase>
    {
    };

    TEST_P(EllipseFContainsPointCircleTests, ContainsPoint)
    {
        const EllipseF ellipse(Vector2(200.0f, 300.0f), 100.0f, 100.0f);
        const auto& c = GetParam();
        EXPECT_EQ(ellipse.Contains(c.x, c.y), c.expected);
    }

    INSTANTIATE_TEST_SUITE_P(EllipseFTest, EllipseFContainsPointCircleTests,
        ::testing::Values(EllipseFContainsCase{-1, -1, false}, EllipseFContainsCase{110, 300, true},
            EllipseFContainsCase{200, 300, true}, EllipseFContainsCase{290, 300, true},
            EllipseFContainsCase{400, 400, false}));

    class EllipseFContainsPointNonCircleTests : public ::testing::TestWithParam<EllipseFContainsCase>
    {
    };

    TEST_P(EllipseFContainsPointNonCircleTests, ContainsPoint)
    {
        const EllipseF ellipse(Vector2(400.0f, 400.0f), 100.0f, 200.0f);
        const auto& c = GetParam();
        EXPECT_EQ(ellipse.Contains(c.x, c.y), c.expected);
    }

    INSTANTIATE_TEST_SUITE_P(EllipseFTest, EllipseFContainsPointNonCircleTests,
        ::testing::Values(EllipseFContainsCase{299, 400, false}, EllipseFContainsCase{501, 400, false},
            EllipseFContainsCase{400, 199, false}, EllipseFContainsCase{400, 601, false},
            EllipseFContainsCase{301, 400, true}, EllipseFContainsCase{499, 400, true},
            EllipseFContainsCase{400, 201, true}, EllipseFContainsCase{400, 599, true}));
}
