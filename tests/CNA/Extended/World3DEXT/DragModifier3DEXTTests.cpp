// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for DragModifier3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/DragModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(DragModifier3DEXTTests, SlowsParticleVelocity)
    {
        DragModifier3DEXT modifier;
        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(100.0f, 0.0f, 0.0f);
        particles[0].MassEXT = 1.0f;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_LT(particles[0].VelocityEXT.X, 100.0f);
    }

    TEST(DragModifier3DEXTTests, Disabled_DoesNotAlterVelocity)
    {
        DragModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        std::vector<Particle3DEXT> particles(1);
        particles[0].VelocityEXT = Vector3(100.0f, 0.0f, 0.0f);

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].VelocityEXT.X, 100.0f);
    }
}
