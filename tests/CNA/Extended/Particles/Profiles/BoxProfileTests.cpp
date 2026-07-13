// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for BoxProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/BoxProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(BoxProfileTests, OffsetLiesOnPerimeter)
    {
        BoxProfile subject;
        subject.Width = 20.0f;
        subject.Height = 10.0f;

        for (int i = 0; i < 50; ++i)
        {
            Vector2 offset;
            Vector2 heading;
            subject.GetOffsetAndHeading(&offset, &heading);

            const bool onVerticalEdge = std::abs(std::abs(offset.X) - 10.0f) < 1e-4f;
            const bool onHorizontalEdge = std::abs(std::abs(offset.Y) - 5.0f) < 1e-4f;
            EXPECT_TRUE(onVerticalEdge || onHorizontalEdge);
            EXPECT_LE(std::abs(offset.X), 10.0f + 1e-4f);
            EXPECT_LE(std::abs(offset.Y), 5.0f + 1e-4f);
        }
    }

    TEST(BoxProfileTests, HeadingIsUnitVector)
    {
        BoxProfile subject;
        subject.Width = 20.0f;
        subject.Height = 10.0f;

        Vector2 offset;
        Vector2 heading;
        subject.GetOffsetAndHeading(&offset, &heading);

        const double length = std::sqrt(static_cast<double>(heading.X) * heading.X + static_cast<double>(heading.Y) * heading.Y);
        EXPECT_NEAR(length, 1.0, 1e-5);
    }
}
