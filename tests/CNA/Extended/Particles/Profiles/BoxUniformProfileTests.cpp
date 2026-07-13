// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for BoxUniformProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/BoxUniformProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(BoxUniformProfileTests, OffsetLiesOnPerimeter)
    {
        BoxUniformProfile subject;
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
        }
    }
}
