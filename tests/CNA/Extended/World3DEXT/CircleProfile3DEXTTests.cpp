// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for CircleProfile3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/CircleProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Random.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(CircleProfile3DEXTTests, OffsetLiesWithinRadius)
    {
        CircleProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = CircleRadiation3DEXT::None;
        System::Random random;

        for (int i = 0; i < 50; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            EXPECT_LE(offset.Length(), 10.0f + 1e-4f);
        }
    }

    TEST(CircleProfile3DEXTTests, RadiateOutHeadingPointsAwayFromCenter)
    {
        CircleProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = CircleRadiation3DEXT::Out;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_GT(Vector3::Dot(offset, heading), 0.0f);
    }

    TEST(CircleProfile3DEXTTests, UnsupportedRadiationModeThrows)
    {
        CircleProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = static_cast<CircleRadiation3DEXT>(99);
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        EXPECT_THROW(subject.GetOffsetAndHeading(&offset, &heading, random), System::ArgumentOutOfRangeException);
    }
}
