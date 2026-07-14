// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for OpacityFastFadeModifier3DEXT (see 3d.md/plan3d.md) -- there is
// no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/OpacityFastFadeModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    TEST(OpacityFastFadeModifier3DEXTTests, SetsOpacityToOneMinusAge)
    {
        OpacityFastFadeModifier3DEXT modifier;
        std::vector<Particle3DEXT> particles(1);
        particles[0].AgeEXT = 0.25f;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].OpacityEXT, 0.75f);
    }

    TEST(OpacityFastFadeModifier3DEXTTests, Disabled_DoesNotAlterOpacity)
    {
        OpacityFastFadeModifier3DEXT modifier;
        modifier.setEnabledProperty(false);
        std::vector<Particle3DEXT> particles(1);
        particles[0].AgeEXT = 0.25f;
        particles[0].OpacityEXT = 0.42f;

        modifier.InternalUpdate(1.0f, particles);

        EXPECT_FLOAT_EQ(particles[0].OpacityEXT, 0.42f);
    }
}
