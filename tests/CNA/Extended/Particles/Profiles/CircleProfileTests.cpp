// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for CircleProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/CircleProfile.hpp"

#include "System/ArgumentOutOfRangeException.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(CircleProfileTests, OffsetLiesWithinRadius)
    {
        CircleProfile subject;
        subject.Radius = 10.0f;
        subject.Radiate = CircleRadiation::None;

        for (int i = 0; i < 50; ++i)
        {
            Vector2 offset;
            Vector2 heading;
            subject.GetOffsetAndHeading(&offset, &heading);

            const double length = std::sqrt(static_cast<double>(offset.X) * offset.X + static_cast<double>(offset.Y) * offset.Y);
            EXPECT_LE(length, 10.0 + 1e-4);
        }
    }

    TEST(CircleProfileTests, RadiateOutHeadingPointsAwayFromCenter)
    {
        CircleProfile subject;
        subject.Radius = 10.0f;
        subject.Radiate = CircleRadiation::Out;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        // Offset should be heading scaled by the sampled distance (both point the same direction).
        const float dot = offset.X * heading.X + offset.Y * heading.Y;
        EXPECT_GT(dot, 0.0f);
    }

    TEST(CircleProfileTests, UnsupportedRadiationModeThrows)
    {
        CircleProfile subject;
        subject.Radius = 10.0f;
        subject.Radiate = static_cast<CircleRadiation>(99);

        Vector2 offset;
        Vector2 heading;
        EXPECT_THROW(subject.GetOffsetAndHeading(&offset, &heading), System::ArgumentOutOfRangeException);
    }
}
