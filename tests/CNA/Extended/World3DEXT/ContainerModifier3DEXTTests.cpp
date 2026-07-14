// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for SphereContainerModifier3DEXT/BoxContainerModifier3DEXT/
// BoxLoopContainerModifier3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/BoxContainerModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/BoxLoopContainerModifier3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SphereContainerModifier3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(SphereContainerModifier3DEXTTests, ReflectsParticleThatExitsInsideBoundary)
    {
        SphereContainerModifier3DEXT modifier;
        modifier.RadiusEXT = 10.0f;
        modifier.InsideEXT = true;
        modifier.RestitutionCoefficientEXT = 1.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(20.0f, 0.0f, 0.0f); // well outside the radius-10 sphere
        particles[0].VelocityEXT = Vector3(5.0f, 0.0f, 0.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_LT(particles[0].VelocityEXT.X, 0.0f);
    }

    TEST(SphereContainerModifier3DEXTTests, ParticleExactlyAtCenter_NoReflectionNoNaN)
    {
        // A-06-style regression: a freshly-emitted PointProfile3DEXT particle starts with
        // PositionEXT == TriggeredPositionEXT (zero local offset) -- must not divide by zero.
        SphereContainerModifier3DEXT modifier;
        modifier.RadiusEXT = 10.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3::Zero;
        particles[0].VelocityEXT = Vector3(1.0f, 2.0f, 3.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_EQ(particles[0].VelocityEXT, Vector3(1.0f, 2.0f, 3.0f));
    }

    TEST(BoxContainerModifier3DEXTTests, ReflectsParticleThatExitsLeftFace)
    {
        BoxContainerModifier3DEXT modifier;
        modifier.WidthEXT = 20;
        modifier.HeightEXT = 20;
        modifier.DepthEXT = 20;
        modifier.RestitutionCoefficientEXT = 1.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(-15.0f, 0.0f, 0.0f); // past the left face at -10
        particles[0].VelocityEXT = Vector3(-5.0f, 0.0f, 0.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_GT(particles[0].PositionEXT.X, -15.0f);
        EXPECT_GT(particles[0].VelocityEXT.X, 0.0f);
    }

    TEST(BoxContainerModifier3DEXTTests, ReflectsParticleThatExitsFrontFace)
    {
        BoxContainerModifier3DEXT modifier;
        modifier.WidthEXT = 20;
        modifier.HeightEXT = 20;
        modifier.DepthEXT = 20;
        modifier.RestitutionCoefficientEXT = 1.0f;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(0.0f, 0.0f, -15.0f); // past the front face at -10
        particles[0].VelocityEXT = Vector3(0.0f, 0.0f, -5.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_GT(particles[0].PositionEXT.Z, -15.0f);
        EXPECT_GT(particles[0].VelocityEXT.Z, 0.0f);
    }

    TEST(BoxLoopContainerModifier3DEXTTests, WrapsParticleToOppositeEdge)
    {
        BoxLoopContainerModifier3DEXT modifier;
        modifier.WidthEXT = 20;
        modifier.HeightEXT = 20;
        modifier.DepthEXT = 20;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(-15.0f, 0.0f, 0.0f); // past the left edge at -10

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].PositionEXT.X, -15.0f + 20.0f);
    }

    TEST(BoxLoopContainerModifier3DEXTTests, WrapsParticleAlongDepthAxis)
    {
        BoxLoopContainerModifier3DEXT modifier;
        modifier.WidthEXT = 20;
        modifier.HeightEXT = 20;
        modifier.DepthEXT = 20;

        std::vector<Particle3DEXT> particles(1);
        particles[0].TriggeredPositionEXT = Vector3::Zero;
        particles[0].PositionEXT = Vector3(0.0f, 0.0f, 15.0f); // past the back edge at 10

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].PositionEXT.Z, 15.0f - 20.0f);
    }
}
