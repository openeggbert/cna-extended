// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Profile3DEXT/PointProfile3DEXT/ConeProfile3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/BoxFillProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/BoxProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/BoxUniformProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CircleProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ConeProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/LineProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/RingProfile3DEXT.hpp"

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

    TEST(Profile3DEXTTests, LineTwoArgFactorySetsAxisAndLength)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Line(Vector3::Right, 5.0f);

        auto* lineProfile = dynamic_cast<LineProfile3DEXT*>(profile.get());
        ASSERT_NE(lineProfile, nullptr);
        EXPECT_EQ(lineProfile->AxisEXT, Vector3::Right);
        EXPECT_FLOAT_EQ(lineProfile->LengthEXT, 5.0f);
        EXPECT_EQ(lineProfile->RadiateEXT, LineRadiation3DEXT::None);
    }

    TEST(Profile3DEXTTests, LineThreeArgFactorySetsRadiateAndZeroesDirection)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Line(Vector3::Right, 5.0f, LineRadiation3DEXT::Perpendicular);

        auto* lineProfile = dynamic_cast<LineProfile3DEXT*>(profile.get());
        ASSERT_NE(lineProfile, nullptr);
        EXPECT_EQ(lineProfile->RadiateEXT, LineRadiation3DEXT::Perpendicular);
        EXPECT_EQ(lineProfile->DirectionEXT, Vector3::Zero);
    }

    TEST(Profile3DEXTTests, LineFourArgFactorySetsDirection)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Line(Vector3::Right, 5.0f, LineRadiation3DEXT::Directional, Vector3::Up);

        auto* lineProfile = dynamic_cast<LineProfile3DEXT*>(profile.get());
        ASSERT_NE(lineProfile, nullptr);
        EXPECT_EQ(lineProfile->RadiateEXT, LineRadiation3DEXT::Directional);
        EXPECT_EQ(lineProfile->DirectionEXT, Vector3::Up);
    }

    TEST(Profile3DEXTTests, RingFactorySetsRadiusAndRadiate)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Ring(7.0f, CircleRadiation3DEXT::Out);

        auto* ringProfile = dynamic_cast<RingProfile3DEXT*>(profile.get());
        ASSERT_NE(ringProfile, nullptr);
        EXPECT_FLOAT_EQ(ringProfile->RadiusEXT, 7.0f);
        EXPECT_EQ(ringProfile->RadiateEXT, CircleRadiation3DEXT::Out);
    }

    TEST(Profile3DEXTTests, CircleFactorySetsRadiusAndRadiate)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Circle(7.0f, CircleRadiation3DEXT::In);

        auto* circleProfile = dynamic_cast<CircleProfile3DEXT*>(profile.get());
        ASSERT_NE(circleProfile, nullptr);
        EXPECT_FLOAT_EQ(circleProfile->RadiusEXT, 7.0f);
        EXPECT_EQ(circleProfile->RadiateEXT, CircleRadiation3DEXT::In);
    }

    TEST(Profile3DEXTTests, BoxFactorySetsDimensions)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::Box(1.0f, 2.0f, 3.0f);

        auto* boxProfile = dynamic_cast<BoxProfile3DEXT*>(profile.get());
        ASSERT_NE(boxProfile, nullptr);
        EXPECT_FLOAT_EQ(boxProfile->WidthEXT, 1.0f);
        EXPECT_FLOAT_EQ(boxProfile->HeightEXT, 2.0f);
        EXPECT_FLOAT_EQ(boxProfile->DepthEXT, 3.0f);
    }

    TEST(Profile3DEXTTests, BoxFillFactorySetsDimensions)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::BoxFill(1.0f, 2.0f, 3.0f);

        auto* boxFillProfile = dynamic_cast<BoxFillProfile3DEXT*>(profile.get());
        ASSERT_NE(boxFillProfile, nullptr);
        EXPECT_FLOAT_EQ(boxFillProfile->WidthEXT, 1.0f);
        EXPECT_FLOAT_EQ(boxFillProfile->HeightEXT, 2.0f);
        EXPECT_FLOAT_EQ(boxFillProfile->DepthEXT, 3.0f);
    }

    TEST(Profile3DEXTTests, BoxUniformFactorySetsDimensions)
    {
        std::unique_ptr<Profile3DEXT> profile = Profile3DEXT::BoxUniform(1.0f, 2.0f, 3.0f);

        auto* boxUniformProfile = dynamic_cast<BoxUniformProfile3DEXT*>(profile.get());
        ASSERT_NE(boxUniformProfile, nullptr);
        EXPECT_FLOAT_EQ(boxUniformProfile->WidthEXT, 1.0f);
        EXPECT_FLOAT_EQ(boxUniformProfile->HeightEXT, 2.0f);
        EXPECT_FLOAT_EQ(boxUniformProfile->DepthEXT, 3.0f);
    }
}
