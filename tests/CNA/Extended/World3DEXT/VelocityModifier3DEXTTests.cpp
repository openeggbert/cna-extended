// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for VelocityModifier3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/VelocityModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/OpacityInterpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(VelocityModifier3DEXTTests, FastParticleGetsFullInterpolatorEffect)
    {
        VelocityModifier3DEXT modifier;
        modifier.VelocityThresholdEXT = 1.0f;
        auto opacityInterpolator = std::make_unique<OpacityInterpolator3DEXT>();
        opacityInterpolator->StartValue = 0.0f;
        opacityInterpolator->EndValue = 1.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(100.0f, 0.0f, 0.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].OpacityEXT, 1.0f);
    }

    TEST(VelocityModifier3DEXTTests, SlowParticleGetsPartialInterpolatorEffect)
    {
        VelocityModifier3DEXT modifier;
        modifier.VelocityThresholdEXT = 10.0f;
        auto opacityInterpolator = std::make_unique<OpacityInterpolator3DEXT>();
        opacityInterpolator->StartValue = 0.0f;
        opacityInterpolator->EndValue = 1.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(5.0f, 0.0f, 0.0f); // half of threshold

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_NEAR(particles[0].OpacityEXT, 0.5f, 1e-4f);
    }
}
