// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for VelocityColorModifier3DEXT (see 3d.md/plan3d.md) -- there is
// no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/VelocityColorModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector3;

    TEST(VelocityColorModifier3DEXTTests, FastParticleGetsVelocityColor)
    {
        VelocityColorModifier3DEXT modifier;
        modifier.VelocityThresholdEXT = 1.0f;
        modifier.VelocityColorEXT = Color(255, 0, 0, 255);
        modifier.StationaryColorEXT = Color(0, 0, 255, 255);

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(100.0f, 0.0f, 0.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(particles[0].ColorEXT.getRProperty(), 255);
        EXPECT_EQ(particles[0].ColorEXT.getBProperty(), 0);
    }

    TEST(VelocityColorModifier3DEXTTests, SlowParticleGetsInterpolatedColor)
    {
        VelocityColorModifier3DEXT modifier;
        modifier.VelocityThresholdEXT = 10.0f;
        modifier.VelocityColorEXT = Color(200, 0, 0, 255);
        modifier.StationaryColorEXT = Color(0, 0, 0, 255);

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(5.0f, 0.0f, 0.0f); // half of threshold

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_NEAR(particles[0].ColorEXT.getRProperty(), 100, 1);
    }

    TEST(VelocityColorModifier3DEXTTests, Disabled_DoesNotAlterColor)
    {
        VelocityColorModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        modifier.VelocityThresholdEXT = 1.0f;
        modifier.VelocityColorEXT = Color(255, 0, 0, 255);

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(100.0f, 0.0f, 0.0f);
        particles[0].ColorEXT = Color(9, 9, 9, 9);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(particles[0].ColorEXT.getRProperty(), 9);
    }
}
