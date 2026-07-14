// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for ScaleInterpolator3DEXT (see 3d.md/plan3d.md) -- there is no
// upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/ScaleInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector2;

    TEST(ScaleInterpolator3DEXTTests, InterpolatesBothAxes)
    {
        ScaleInterpolator3DEXT subject;
        subject.StartValue = Vector2(0.0f, 1.0f);
        subject.EndValue = Vector2(2.0f, 3.0f);

        Particle3DEXT particle;
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.ScaleEXT.X, 1.0f);
        EXPECT_FLOAT_EQ(particle.ScaleEXT.Y, 2.0f);
    }

    TEST(ScaleInterpolator3DEXTTests, Disabled_DoesNothing)
    {
        ScaleInterpolator3DEXT subject;
        subject.setEnabledProperty(false);
        subject.StartValue = Vector2(0.0f, 1.0f);
        subject.EndValue = Vector2(2.0f, 3.0f);

        Particle3DEXT particle;
        particle.ScaleEXT = Vector2(9.0f, 9.0f);
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.ScaleEXT.X, 9.0f);
        EXPECT_FLOAT_EQ(particle.ScaleEXT.Y, 9.0f);
    }
}
