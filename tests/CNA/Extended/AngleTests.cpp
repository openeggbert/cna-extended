// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/AngleTest.cs. Upstream's
// WithinDeltaEqualityComparer/EqualsWithTolerence test helpers have no port here (test-only
// infrastructure, not part of Angle.cs itself); replaced with GoogleTest's EXPECT_NEAR at the
// same delta (0.00001f).
#include "CNA/Extended/Angle.hpp"

#include <gtest/gtest.h>

#include "Microsoft/Xna/Framework/MathHelper.hpp"

namespace CNA::Extended
{
    namespace
    {
        constexpr float kDelta = 0.00001f;

        using Microsoft::Xna::Framework::MathHelper;
    }

    TEST(AngleTests, ConstructorTest)
    {
        constexpr float value = 0.5f;

        const Angle radians(value, AngleType::Radian);
        const Angle degrees(value, AngleType::Degree);
        const Angle gradians(value, AngleType::Gradian);
        const Angle revolutions(value, AngleType::Revolution);

        EXPECT_NEAR(radians.getRadiansProperty(), 0.5f, kDelta);
        EXPECT_NEAR(degrees.getDegreesProperty(), 0.5f, kDelta);
        EXPECT_NEAR(gradians.getGradiansProperty(), 0.5f, kDelta);
        EXPECT_NEAR(revolutions.getRevolutionsProperty(), 0.5f, kDelta);
    }

    TEST(AngleTests, ConversionTest)
    {
        // from radians
        const Angle radians(MathHelper::Pi);
        EXPECT_NEAR(radians.getDegreesProperty(), 180.0f, kDelta);
        EXPECT_NEAR(radians.getGradiansProperty(), 200.0f, kDelta);
        EXPECT_NEAR(radians.getRevolutionsProperty(), 0.5f, kDelta);

        // to radians
        const Angle degrees(180.0f, AngleType::Degree);
        const Angle gradians(200.0f, AngleType::Gradian);
        const Angle revolutions(0.5f, AngleType::Revolution);

        EXPECT_NEAR(degrees.getRadiansProperty(), MathHelper::Pi, kDelta);
        EXPECT_NEAR(gradians.getRadiansProperty(), MathHelper::Pi, kDelta);
        EXPECT_NEAR(revolutions.getRadiansProperty(), MathHelper::Pi, kDelta);
    }

    TEST(AngleTests, WrapTest)
    {
        for (float f = -10.0f; f < 10.0f; f += 0.1f)
        {
            Angle wrapPositive(f);
            wrapPositive.WrapPositive();

            Angle wrap(f);
            wrap.Wrap();

            EXPECT_GE(wrapPositive.getRadiansProperty(), 0.0f);
            EXPECT_LT(wrapPositive.getRadiansProperty(), 2.0f * MathHelper::Pi);

            EXPECT_GE(wrap.getRadiansProperty(), -MathHelper::Pi);
            EXPECT_LT(wrap.getRadiansProperty(), MathHelper::Pi);
        }
    }

    TEST(AngleTests, VectorTest)
    {
        Angle angle = Angle::FromVector(Vector2::One);
        EXPECT_NEAR(angle.getRadiansProperty(), -MathHelper::Pi / 4.0f, kDelta);
        EXPECT_FLOAT_EQ(angle.ToVector(10.0f).Length(), 10.0f);

        angle = Angle::FromVector(Vector2::UnitX);
        EXPECT_NEAR(angle.getRadiansProperty(), 0.0f, kDelta);
        EXPECT_NEAR(angle.ToUnitVector().X, Vector2::UnitX.X, kDelta);
        EXPECT_NEAR(angle.ToUnitVector().Y, Vector2::UnitX.Y, kDelta);

        angle = Angle::FromVector(-Vector2::UnitY);
        EXPECT_NEAR(angle.getRadiansProperty(), MathHelper::Pi / 2.0f, kDelta);
        EXPECT_NEAR(angle.ToUnitVector().X, (-Vector2::UnitY).X, kDelta);
        EXPECT_NEAR(angle.ToUnitVector().Y, (-Vector2::UnitY).Y, kDelta);
    }

    TEST(AngleTests, EqualsTest)
    {
        Angle angle1(0.0f);
        Angle angle2(MathHelper::Pi * 2.0f);
        EXPECT_TRUE(angle1 == angle2);
        angle2.setRadiansProperty(MathHelper::Pi * 4.0f);
        EXPECT_TRUE(angle1.Equals(angle2));
    }
}
