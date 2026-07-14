// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for HueInterpolator3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/HueInterpolator3DEXT.hpp"

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;

    TEST(HueInterpolator3DEXTTests, InterpolatesHueAtHalfway)
    {
        HueInterpolator3DEXT subject;
        subject.StartValue = 0.0f; // red
        subject.EndValue = 240.0f; // blue

        Particle3DEXT particle;
        particle.ColorEXT = Color(255, 0, 0, 255); // pure red, hue 0

        subject.Update(0.5f, &particle);

        const HslColor result = HslColor::FromRgb(particle.ColorEXT);
        EXPECT_NEAR(result.getHProperty(), 120.0f, 5.0f); // roughly green
    }

    TEST(HueInterpolator3DEXTTests, PreservesSaturationAndLightnessApproximately)
    {
        HueInterpolator3DEXT subject;
        const Color original(200, 100, 50, 255);
        const HslColor originalHsl = HslColor::FromRgb(original);
        subject.StartValue = originalHsl.getHProperty();
        subject.EndValue = originalHsl.getHProperty();

        Particle3DEXT particle;
        particle.ColorEXT = original;

        subject.Update(0.5f, &particle);

        const HslColor result = HslColor::FromRgb(particle.ColorEXT);
        EXPECT_NEAR(result.getSProperty(), originalHsl.getSProperty(), 0.02f);
        EXPECT_NEAR(result.getLProperty(), originalHsl.getLProperty(), 0.02f);
    }

    TEST(HueInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        HueInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = 0.0f;
        subject.EndValue = 240.0f;

        Particle3DEXT particle;
        particle.ColorEXT = Color(255, 0, 0, 255);

        subject.Update(0.5f, &particle);

        EXPECT_EQ(particle.ColorEXT.getRProperty(), 255);
        EXPECT_EQ(particle.ColorEXT.getGProperty(), 0);
        EXPECT_EQ(particle.ColorEXT.getBProperty(), 0);
    }
}
