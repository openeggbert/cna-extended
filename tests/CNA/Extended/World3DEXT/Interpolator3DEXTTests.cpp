// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Interpolator3DEXT (base class)/InterpolatorOfT3DEXT<T>,
// ColorInterpolator3DEXT, and OpacityInterpolator3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/Interpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ColorInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/OpacityInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;

    TEST(Interpolator3DEXTTests, NameDefaultsToConstructorArgument)
    {
        OpacityInterpolator3DEXT subject;
        EXPECT_EQ(subject.getNameProperty(), "OpacityInterpolator3DEXT");
    }

    TEST(Interpolator3DEXTTests, SetName_UpdatesValue)
    {
        OpacityInterpolator3DEXT subject;
        subject.setNameProperty("Renamed");
        EXPECT_EQ(subject.getNameProperty(), "Renamed");
    }

    TEST(Interpolator3DEXTTests, EnabledDefaultsToTrue)
    {
        OpacityInterpolator3DEXT subject;
        EXPECT_TRUE(subject.getEnabledProperty());
    }

    TEST(ColorInterpolator3DEXTTests, DefaultStartAndEndValuesAreWhite)
    {
        ColorInterpolator3DEXT subject;
        EXPECT_EQ(subject.StartValue, Color::White);
        EXPECT_EQ(subject.EndValue, Color::White);
    }

    TEST(ColorInterpolator3DEXTTests, InterpolatesRgbAtHalfway)
    {
        ColorInterpolator3DEXT subject;
        subject.StartValue = Color(0, 0, 0, 255);
        subject.EndValue = Color(200, 100, 50, 255);

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_NEAR(particle.ColorEXT.getRProperty(), 100, 1);
        EXPECT_NEAR(particle.ColorEXT.getGProperty(), 50, 1);
        EXPECT_NEAR(particle.ColorEXT.getBProperty(), 25, 1);
    }

    TEST(ColorInterpolator3DEXTTests, AlwaysEmitsFullAlpha)
    {
        ColorInterpolator3DEXT subject;
        subject.StartValue = Color(0, 0, 0, 10);
        subject.EndValue = Color(200, 100, 50, 20);

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_EQ(particle.ColorEXT.getAProperty(), 255);
    }

    TEST(ColorInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        ColorInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = Color(0, 0, 0, 255);
        subject.EndValue = Color(200, 100, 50, 255);

        Particle3DEXT particle;
        particle.ColorEXT = Color(9, 9, 9, 9);
        subject.Update(0.5f, &particle);

        EXPECT_EQ(particle.ColorEXT.getRProperty(), 9);
    }

    TEST(OpacityInterpolator3DEXTTests, DefaultStartAndEndValuesMatchEmitterDefaults)
    {
        OpacityInterpolator3DEXT subject;
        EXPECT_FLOAT_EQ(subject.StartValue, 1.0f);
        EXPECT_FLOAT_EQ(subject.EndValue, 0.0f);
    }

    TEST(OpacityInterpolator3DEXTTests, InterpolatesOpacity)
    {
        OpacityInterpolator3DEXT subject;
        subject.StartValue = 1.0f;
        subject.EndValue = 0.0f;

        Particle3DEXT particle;
        subject.Update(0.75f, &particle);

        EXPECT_FLOAT_EQ(particle.OpacityEXT, 0.25f);
    }

    // A-06 (audit.md): out-of-[0,1] results must be clamped, not passed through raw.
    TEST(OpacityInterpolator3DEXTTests, ClampsOutOfRangeResult)
    {
        OpacityInterpolator3DEXT subject;
        subject.StartValue = 2.0f;
        subject.EndValue = -1.0f;

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_GE(particle.OpacityEXT, 0.0f);
        EXPECT_LE(particle.OpacityEXT, 1.0f);
    }

    TEST(OpacityInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        OpacityInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = 1.0f;
        subject.EndValue = 0.0f;

        Particle3DEXT particle;
        particle.OpacityEXT = 0.42f;
        subject.Update(0.75f, &particle);

        EXPECT_FLOAT_EQ(particle.OpacityEXT, 0.42f);
    }
}
