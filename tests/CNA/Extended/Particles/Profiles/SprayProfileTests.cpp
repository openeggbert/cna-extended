// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for SprayProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/SprayProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(SprayProfileTests, ReturnsZeroOffset)
    {
        SprayProfile subject;
        subject.Direction = Vector2(1.0f, 0.0f);
        subject.Spread = 0.5f;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_FLOAT_EQ(offset.X, 0.0f);
        EXPECT_FLOAT_EQ(offset.Y, 0.0f);
    }

    TEST(SprayProfileTests, ZeroSpreadAlwaysReturnsExactDirection)
    {
        SprayProfile subject;
        subject.Direction = Vector2(1.0f, 0.0f);
        subject.Spread = 0.0f;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_NEAR(heading.X, 1.0f, 1e-5f);
        EXPECT_NEAR(heading.Y, 0.0f, 1e-5f);
    }

    TEST(SprayProfileTests, HeadingIsUnitVector)
    {
        SprayProfile subject;
        subject.Direction = Vector2(1.0f, 1.0f);
        subject.Spread = 1.0f;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        const double length = std::sqrt(static_cast<double>(heading.X) * heading.X + static_cast<double>(heading.Y) * heading.Y);
        EXPECT_NEAR(length, 1.0, 1e-5);
    }
}
