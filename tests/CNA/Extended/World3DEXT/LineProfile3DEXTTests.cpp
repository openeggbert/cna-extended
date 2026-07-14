// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for LineProfile3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/LineProfile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Random.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(LineProfile3DEXTTests, OffsetLiesAlongAxisWithinHalfLength)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 20.0f;
        subject.RadiateEXT = LineRadiation3DEXT::None;
        System::Random random;

        for (int i = 0; i < 50; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            EXPECT_LE(std::abs(offset.X), 10.0f + 1e-4f);
            EXPECT_NEAR(offset.Y, 0.0f, 1e-4f);
            EXPECT_NEAR(offset.Z, 0.0f, 1e-4f);
        }
    }

    TEST(LineProfile3DEXTTests, NoneRadiateProducesUnitHeading)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 10.0f;
        subject.RadiateEXT = LineRadiation3DEXT::None;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }

    TEST(LineProfile3DEXTTests, DirectionalRadiateUsesNormalizedDirection)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 10.0f;
        subject.RadiateEXT = LineRadiation3DEXT::Directional;
        subject.DirectionEXT = Vector3(0.0f, 5.0f, 0.0f);
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.X, 0.0f, 1e-5f);
        EXPECT_NEAR(heading.Y, 1.0f, 1e-5f);
        EXPECT_NEAR(heading.Z, 0.0f, 1e-5f);
    }

    TEST(LineProfile3DEXTTests, PerpendicularRadiateIsPerpendicularToAxis)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 10.0f;
        subject.RadiateEXT = LineRadiation3DEXT::Perpendicular;
        System::Random random;

        for (int i = 0; i < 50; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            EXPECT_NEAR(Vector3::Dot(Vector3::Normalize(subject.AxisEXT), heading), 0.0f, 1e-4f);
            EXPECT_NEAR(heading.Length(), 1.0f, 1e-4f);
        }
    }

    // A-06-style regression: a zero AxisEXT/DirectionEXT must not propagate NaN.
    TEST(LineProfile3DEXTTests, ZeroAxis_ProducesFiniteOffset)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3::Zero;
        subject.LengthEXT = 10.0f;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_TRUE(std::isfinite(offset.X));
        EXPECT_TRUE(std::isfinite(offset.Y));
        EXPECT_TRUE(std::isfinite(offset.Z));
    }

    TEST(LineProfile3DEXTTests, ZeroDirection_DirectionalRadiate_ProducesFiniteHeading)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 10.0f;
        subject.RadiateEXT = LineRadiation3DEXT::Directional;
        subject.DirectionEXT = Vector3::Zero;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_TRUE(std::isfinite(heading.X));
        EXPECT_TRUE(std::isfinite(heading.Y));
        EXPECT_TRUE(std::isfinite(heading.Z));
    }

    TEST(LineProfile3DEXTTests, UnsupportedRadiationModeThrows)
    {
        LineProfile3DEXT subject;
        subject.AxisEXT = Vector3(1.0f, 0.0f, 0.0f);
        subject.LengthEXT = 10.0f;
        subject.RadiateEXT = static_cast<LineRadiation3DEXT>(99);
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        EXPECT_THROW(subject.GetOffsetAndHeading(&offset, &heading, random), System::InvalidOperationException);
    }
}
