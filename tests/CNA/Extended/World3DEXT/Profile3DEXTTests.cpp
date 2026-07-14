// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Profile3DEXT/PointProfile3DEXT/ConeProfile3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/ConeProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(PointProfile3DEXTTests, ReturnsZeroOffset)
    {
        PointProfile3DEXT subject;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_FLOAT_EQ(offset.X, 0.0f);
        EXPECT_FLOAT_EQ(offset.Y, 0.0f);
        EXPECT_FLOAT_EQ(offset.Z, 0.0f);
    }

    TEST(PointProfile3DEXTTests, ReturnsHeadingAsUnitVector)
    {
        PointProfile3DEXT subject;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }

    TEST(ConeProfile3DEXTTests, ReturnsZeroOffset)
    {
        ConeProfile3DEXT subject;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_FLOAT_EQ(offset.X, 0.0f);
        EXPECT_FLOAT_EQ(offset.Y, 0.0f);
        EXPECT_FLOAT_EQ(offset.Z, 0.0f);
    }

    TEST(ConeProfile3DEXTTests, ReturnsHeadingAsUnitVector)
    {
        ConeProfile3DEXT subject;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_NEAR(heading.Length(), 1.0f, 1e-5f);
    }

    TEST(ConeProfile3DEXTTests, HeadingStaysWithinHalfAngleOfDirection)
    {
        ConeProfile3DEXT subject;
        subject.DirectionEXT = Vector3::Up;
        subject.HalfAngleEXT = 0.1f; // narrow cone
        System::Random random;

        for (int i = 0; i < 50; ++i)
        {
            Vector3 offset;
            Vector3 heading;
            subject.GetOffsetAndHeading(&offset, &heading, random);

            const float cosAngle = Vector3::Dot(Vector3::Normalize(subject.DirectionEXT), heading);
            EXPECT_GE(cosAngle, std::cos(subject.HalfAngleEXT) - 1e-4f);
        }
    }

    // A-06 regression (audit.md): a zero DirectionEXT must not propagate NaN through
    // Vector3::Normalize -- falls back to Vector3::Up instead (see ConeProfile3DEXT.cpp).
    TEST(ConeProfile3DEXTTests, ZeroDirection_ProducesFiniteHeading)
    {
        ConeProfile3DEXT subject;
        subject.DirectionEXT = Vector3::Zero;
        System::Random random;

        Vector3 offset;
        Vector3 heading;
        subject.GetOffsetAndHeading(&offset, &heading, random);

        EXPECT_TRUE(std::isfinite(heading.X));
        EXPECT_TRUE(std::isfinite(heading.Y));
        EXPECT_TRUE(std::isfinite(heading.Z));
    }

    TEST(Profile3DEXTTests, PointFactoryCreatesPointProfile)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Point();

        EXPECT_NE(dynamic_cast<PointProfile3DEXT*>(profile.get()), nullptr);
    }

    TEST(Profile3DEXTTests, ConeFactoryCreatesConeProfileWithGivenFields)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Cone(Vector3::Right, 0.5f);

        auto* coneProfile = dynamic_cast<ConeProfile3DEXT*>(profile.get());
        ASSERT_NE(coneProfile, nullptr);
        EXPECT_EQ(coneProfile->DirectionEXT, Vector3::Right);
        EXPECT_FLOAT_EQ(coneProfile->HalfAngleEXT, 0.5f);
    }
}
