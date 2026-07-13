// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for BoxFillProfile. Fresh tests below.
#include "CNA/Extended/Particles/Profiles/BoxFillProfile.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Profiles
{
    TEST(BoxFillProfileTests, OffsetLiesWithinArea)
    {
        BoxFillProfile subject;
        subject.Width = 20.0f;
        subject.Height = 10.0f;

        for (int i = 0; i < 50; ++i)
        {
            Vector2 offset;
            Vector2 heading;
            subject.GetOffsetAndHeading(&offset, &heading);

            EXPECT_LE(std::abs(offset.X), 10.0f);
            EXPECT_LE(std::abs(offset.Y), 5.0f);
        }
    }
}
