// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for LineProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/LineProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(LineProfileTests, OffsetLiesAlongAxisWithinHalfLength)
    {
        LineProfile subject;
        subject.Axis = Vector2(1.0f, 0.0f);
        subject.Length = 20.0f;
        subject.Radiate = LineRadiation::None;

        for (int i = 0; i < 50; ++i)
        {
            Vector2 offset;
            Vector2 heading;
            subject.GetOffsetAndHeading(&offset, &heading);

            EXPECT_LE(std::abs(offset.X), 10.0f + 1e-4f);
            EXPECT_NEAR(offset.Y, 0.0f, 1e-4f);
        }
    }

    TEST(LineProfileTests, DirectionalRadiateUsesNormalizedDirection)
    {
        LineProfile subject;
        subject.Axis = Vector2(1.0f, 0.0f);
        subject.Length = 10.0f;
        subject.Radiate = LineRadiation::Directional;
        subject.Direction = Vector2(0.0f, 5.0f);

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_NEAR(heading.X, 0.0f, 1e-5f);
        EXPECT_NEAR(heading.Y, 1.0f, 1e-5f);
    }

    TEST(LineProfileTests, PerpendicularUpIsPerpendicularToAxis)
    {
        LineProfile subject;
        subject.Axis = Vector2(1.0f, 0.0f);
        subject.Length = 10.0f;
        subject.Radiate = LineRadiation::PerpendicularUp;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        EXPECT_NEAR(heading.X, 0.0f, 1e-5f);
        EXPECT_NEAR(heading.Y, -1.0f, 1e-5f);
    }
}
