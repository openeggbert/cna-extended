// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for BoxFillProfile3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/BoxFillProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(BoxFillProfile3DEXTTests, OffsetLiesWithinVolume)
    {
        BoxFillProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        for (int i = 0; i < 50; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            EXPECT_LE(std::abs(offset.X), 10.0f);
            EXPECT_LE(std::abs(offset.Y), 5.0f);
            EXPECT_LE(std::abs(offset.Z), 3.0f);
        }
    }

    TEST(BoxFillProfile3DEXTTests, HeadingIsUnitVector)
    {
        BoxFillProfile3DEXT subject;
        subject.WidthEXT = 20.0f;
        subject.HeightEXT = 10.0f;
        subject.DepthEXT = 6.0f;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }
}
