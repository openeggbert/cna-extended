// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for VortexModifier3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/VortexModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(VortexModifier3DEXTTests, PullsParticleTowardAxisWithinRadii)
    {
        VortexModifier3DEXT modifier;
        modifier.AxisEXT = Vector3::Up;
        modifier.PositionEXT = Vector3::Zero;
        modifier.StrengthEXT = 100.0f;
        modifier.InnerRadiusEXT = 0.0f;
        modifier.OuterRadiusEXT = 100.0f;
        modifier.MaxVelocityEXT = 1000.0f;
        modifier.setRotationAngleEXTProperty(0.0f);

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(10.0f, 0.0f, 0.0f);
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        // Pure attraction (angle 0) should accelerate the particle toward the axis line.
        EXPECT_LT(particles[0].VelocityEXT.X, 0.0f);
    }

    TEST(VortexModifier3DEXTTests, SetRotationAngleEXTRecomputesCachedTrig)
    {
        VortexModifier3DEXT modifier;
        modifier.setRotationAngleEXTProperty(1.5f);
        EXPECT_FLOAT_EQ(modifier.getRotationAngleEXTProperty(), 1.5f);
    }

    TEST(VortexModifier3DEXTTests, OffsetPurelyAlongAxisHasNoRadialDistance_NoForceApplied)
    {
        // A particle displaced only along the axis (zero perpendicular/radial component) has
        // distance == 0 from the axis LINE -- physically correct for a line-vortex to leave it
        // untouched (also exercises the A-06-style near-zero-distance guard).
        VortexModifier3DEXT modifier;
        modifier.AxisEXT = Vector3::Up;
        modifier.PositionEXT = Vector3::Zero;
        modifier.StrengthEXT = 100.0f;
        modifier.InnerRadiusEXT = 0.0f;
        modifier.OuterRadiusEXT = 100.0f;
        modifier.MaxVelocityEXT = 1000.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(0.0f, 5.0f, 0.0f); // purely along AxisEXT
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(particles[0].VelocityEXT, Vector3::Zero);
    }

    TEST(VortexModifier3DEXTTests, OutsideOuterRadiusIsUnaffected)
    {
        VortexModifier3DEXT modifier;
        modifier.AxisEXT = Vector3::Up;
        modifier.StrengthEXT = 100.0f;
        modifier.InnerRadiusEXT = 0.0f;
        modifier.OuterRadiusEXT = 5.0f;
        modifier.MaxVelocityEXT = 1000.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(50.0f, 0.0f, 0.0f); // well beyond OuterRadiusEXT
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(particles[0].VelocityEXT, Vector3::Zero);
    }

    TEST(VortexModifier3DEXTTests, VelocityIsClampedToMaxVelocityEXT)
    {
        VortexModifier3DEXT modifier;
        modifier.AxisEXT = Vector3::Up;
        modifier.StrengthEXT = 100000.0f;
        modifier.InnerRadiusEXT = 0.0f;
        modifier.OuterRadiusEXT = 100.0f;
        modifier.MaxVelocityEXT = 5.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(10.0f, 0.0f, 0.0f);
        particles[0].VelocityEXT = Vector3::Zero;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_NEAR(particles[0].VelocityEXT.Length(), 5.0f, 1e-3f);
    }
}
