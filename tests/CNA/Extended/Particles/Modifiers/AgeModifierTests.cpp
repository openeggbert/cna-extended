// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for AgeModifier/VelocityModifier. Fresh tests below.
#include "CNA/Extended/Particles/Modifiers/AgeModifier.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/OpacityInterpolator.hpp"
#include "CNA/Extended/Particles/Modifiers/VelocityModifier.hpp"
#include "CNA/Extended/Particles/ParticleBuffer.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Particles::Modifiers
{
    TEST(AgeModifierTests, AppliesInterpolatorsBasedOnParticleAge)
    {
        AgeModifier modifier;
        auto opacityInterpolator = std::make_unique<Interpolators::OpacityInterpolator>();
        opacityInterpolator->StartValue = 1.0f;
        opacityInterpolator->EndValue = 0.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        ParticleBuffer buffer(1);
        ParticleIterator& releaseIterator = buffer.Release(1);
        Data::Particle* particle = releaseIterator.Next();
        particle->Age = 0.5f;

        ParticleIterator& updateIterator = buffer.getIteratorProperty();
        modifier.InternalUpdate(1.0f, updateIterator);

        EXPECT_FLOAT_EQ(particle->Opacity, 0.5f);
    }

    TEST(VelocityModifierTests, FastParticleGetsFullInterpolatorEffect)
    {
        VelocityModifier modifier;
        modifier.VelocityThreshold = 1.0f;
        auto opacityInterpolator = std::make_unique<Interpolators::OpacityInterpolator>();
        opacityInterpolator->StartValue = 0.0f;
        opacityInterpolator->EndValue = 1.0f;
        modifier.getInterpolatorsProperty().push_back(std::move(opacityInterpolator));

        ParticleBuffer buffer(1);
        ParticleIterator& releaseIterator = buffer.Release(1);
        Data::Particle* particle = releaseIterator.Next();
        particle->Velocity[0] = 100.0f;
        particle->Velocity[1] = 0.0f;

        ParticleIterator& updateIterator = buffer.getIteratorProperty();
        modifier.InternalUpdate(1.0f, updateIterator);

        EXPECT_FLOAT_EQ(particle->Opacity, 1.0f);
    }
}
