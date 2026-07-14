// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for RingProfile3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/RingProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/Random.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(RingProfile3DEXTTests, ReturnsOffsetEqualToRadius)
    {
        RingProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(offset.Length(), 10.0f, 1e-4f);
    }

    TEST(RingProfile3DEXTTests, WhenRadiateIsOutHeadingIsEqualToNormalizedOffset)
    {
        RingProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = CircleRadiation3DEXT::Out;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.X, offset.X / 10.0f, 1e-4f);
        EXPECT_NEAR(heading.Y, offset.Y / 10.0f, 1e-4f);
        EXPECT_NEAR(heading.Z, offset.Z / 10.0f, 1e-4f);
    }

    TEST(RingProfile3DEXTTests, WhenRadiateIsInOffsetPointsOppositeHeading)
    {
        RingProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = CircleRadiation3DEXT::In;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(Vector3::Dot(offset, heading), -10.0f, 1e-2f);
    }

    TEST(RingProfile3DEXTTests, UnsupportedRadiationModeThrows)
    {
        RingProfile3DEXT subject;
        subject.RadiusEXT = 10.0f;
        subject.RadiateEXT = static_cast<CircleRadiation3DEXT>(99);
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        EXPECT_THROW(subject.GetOffsetAndHeading(&offset, &heading, random), System::ArgumentOutOfRangeException);
    }
}
