// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for VelocityInterpolator3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/VelocityInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    TEST(VelocityInterpolator3DEXTTests, InterpolatesAllThreeAxes)
    {
        VelocityInterpolator3DEXT subject;
        subject.StartValue = Vector3(0.0f, 0.0f, 0.0f);
        subject.EndValue = Vector3(10.0f, -10.0f, 20.0f);

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.VelocityEXT.X, 5.0f);
        EXPECT_FLOAT_EQ(particle.VelocityEXT.Y, -5.0f);
        EXPECT_FLOAT_EQ(particle.VelocityEXT.Z, 10.0f);
    }

    TEST(VelocityInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        VelocityInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = Vector3(0.0f, 0.0f, 0.0f);
        subject.EndValue = Vector3(10.0f, -10.0f, 20.0f);

        Particle3DEXT particle;
        particle.VelocityEXT = Vector3(9.0f, 9.0f, 9.0f);
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.VelocityEXT.X, 9.0f);
        EXPECT_FLOAT_EQ(particle.VelocityEXT.Y, 9.0f);
        EXPECT_FLOAT_EQ(particle.VelocityEXT.Z, 9.0f);
    }
}
