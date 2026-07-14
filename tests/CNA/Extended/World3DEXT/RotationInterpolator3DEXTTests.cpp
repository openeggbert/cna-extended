// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for RotationInterpolator3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/RotationInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    TEST(RotationInterpolator3DEXTTests, InterpolatesRotation)
    {
        RotationInterpolator3DEXT subject;
        subject.StartValue = 0.0f;
        subject.EndValue = 4.0f;

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.RotationEXT, 2.0f);
    }

    TEST(RotationInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        RotationInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = 0.0f;
        subject.EndValue = 4.0f;

        Particle3DEXT particle;
        particle.RotationEXT = 0.5f;
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.RotationEXT, 0.5f);
    }
}
