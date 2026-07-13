// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Particles/Profiles/PointProfileTests.cs.
#include "CNA/Extended/Particles/Profiles/PointProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(PointProfileTests, ReturnsZeroOffset)
    {
        PointProfile subject;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_FLOAT_EQ(offset.X, 0.0f);
        EXPECT_FLOAT_EQ(offset.Y, 0.0f);
    }

    TEST(PointProfileTests, ReturnsHeadingAsUnitVector)
    {
        PointProfile subject;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        const double length = std::sqrt(static_cast<double>(heading.X) * heading.X + static_cast<double>(heading.Y) * heading.Y);
        EXPECT_NEAR(length, 1.0, 1e-6);
    }
}
