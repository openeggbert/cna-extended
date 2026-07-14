// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for RotationModifier3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/RotationModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    TEST(RotationModifier3DEXTTests, IncreasesRotationOverTime)
    {
        RotationModifier3DEXT modifier;
        modifier.RotationRateEXT = 1.0f;
        std::vector<Particle3DEXT> particles(1);
        particles[0].RotationEXT = 0.0f;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_GT(particles[0].RotationEXT, 0.0f);
    }

    TEST(RotationModifier3DEXTTests, Disabled_DoesNotAlterRotation)
    {
        RotationModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        modifier.RotationRateEXT = 1.0f;
        std::vector<Particle3DEXT> particles(1);
        particles[0].RotationEXT = 0.0f;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].RotationEXT, 0.0f);
    }
}
