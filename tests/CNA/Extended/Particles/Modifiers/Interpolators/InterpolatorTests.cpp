// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for these Interpolator subclasses. Fresh tests below, calling
// Interpolator::Update() directly (public, unlike Modifier::Update) against a plain stack-allocated
// Data::Particle -- no ParticleBuffer/iterator needed here since interpolators don't touch buffer state.
#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ColorInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/HueInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/OpacityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/RotationInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/ScaleInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/VelocityInterpolator.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    namespace
    {
        Data::Particle MakeParticle()
        {
            Data::Particle particle{};
            return particle;
        }
    }

    TEST(ColorInterpolatorTests, InterpolatesHslAtHalfway)
    {
        ColorInterpolator subject;
        subject.StartValue = HslColor(0.0f, 0.0f, 0.0f);
        subject.EndValue = HslColor(1.0f, 1.0f, 1.0f);

        Data::Particle particle = MakeParticle();
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.Color[0], 0.5f);
        EXPECT_FLOAT_EQ(particle.Color[1], 0.5f);
        EXPECT_FLOAT_EQ(particle.Color[2], 0.5f);
    }

    TEST(ColorInterpolatorTests, DisabledDoesNothing)
    {
        ColorInterpolator subject;
        subject.setEnabledProperty(false);
        subject.StartValue = HslColor(0.0f, 0.0f, 0.0f);
        subject.EndValue = HslColor(1.0f, 1.0f, 1.0f);

        Data::Particle particle = MakeParticle();
        particle.Color[0] = 0.25f;
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.Color[0], 0.25f);
    }

    TEST(HueInterpolatorTests, InterpolatesOnlyHue)
    {
        HueInterpolator subject;
        subject.StartValue = 0.0f;
        subject.EndValue = 1.0f;

        Data::Particle particle = MakeParticle();
        subject.Update(0.25f, &particle);

        EXPECT_FLOAT_EQ(particle.Color[0], 0.25f);
    }

    TEST(OpacityInterpolatorTests, InterpolatesOpacity)
    {
        OpacityInterpolator subject;
        subject.StartValue = 1.0f;
        subject.EndValue = 0.0f;

        Data::Particle particle = MakeParticle();
        subject.Update(0.75f, &particle);

        EXPECT_FLOAT_EQ(particle.Opacity, 0.25f);
    }

    TEST(RotationInterpolatorTests, InterpolatesRotation)
    {
        RotationInterpolator subject;
        subject.StartValue = 0.0f;
        subject.EndValue = 4.0f;

        Data::Particle particle = MakeParticle();
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.Rotation, 2.0f);
    }

    TEST(ScaleInterpolatorTests, InterpolatesBothAxes)
    {
        ScaleInterpolator subject;
        subject.StartValue = Vector2(0.0f, 1.0f);
        subject.EndValue = Vector2(2.0f, 3.0f);

        Data::Particle particle = MakeParticle();
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.Scale[0], 1.0f);
        EXPECT_FLOAT_EQ(particle.Scale[1], 2.0f);
    }

    TEST(VelocityInterpolatorTests, InterpolatesBothAxes)
    {
        VelocityInterpolator subject;
        subject.StartValue = Vector2(0.0f, 0.0f);
        subject.EndValue = Vector2(10.0f, -10.0f);

        Data::Particle particle = MakeParticle();
        subject.Update(0.5f, &particle);

        EXPECT_FLOAT_EQ(particle.Velocity[0], 5.0f);
        EXPECT_FLOAT_EQ(particle.Velocity[1], -5.0f);
    }
}
