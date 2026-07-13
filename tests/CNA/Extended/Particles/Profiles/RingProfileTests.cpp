// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Particles/Profiles/RingProfileTests.cs.
#include "CNA/Extended/Particles/Profiles/RingProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(RingProfileTests, ReturnsOffsetEqualToRadius)
    {
        RingProfile subject;
        subject.Radius = 10.0f;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        const double length = std::sqrt(static_cast<double>(offset.X) * offset.X + static_cast<double>(offset.Y) * offset.Y);
        EXPECT_NEAR(length, 10.0, 1e-5);
    }

    TEST(RingProfileTests, WhenRadiateIsOutHeadingIsEqualToNormalizedOffset)
    {
        RingProfile subject;
        subject.Radius = 10.0f;
        subject.Radiate = CircleRadiation::Out;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_NEAR(heading.X, offset.X / 10.0f, 1e-5);
        EXPECT_NEAR(heading.Y, offset.Y / 10.0f, 1e-5);
    }
}
