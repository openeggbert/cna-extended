// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/VelocityModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

#include <cmath>

namespace CNA::Extended::Particles::Modifiers
{
    void VelocityModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        const float velocityThreshold2 = VelocityThreshold * VelocityThreshold;

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            const float velocitySquared = particle->Velocity[0] * particle->Velocity[0] + particle->Velocity[1] * particle->Velocity[1];

            if (velocitySquared >= velocityThreshold2)
            {
                for (const std::unique_ptr<Interpolators::Interpolator>& interpolator : interpolators_)
                {
                    interpolator->Update(1.0f, particle);
                }
            }
            else
            {
                const float t = std::sqrt(velocitySquared) / VelocityThreshold;

                for (const std::unique_ptr<Interpolators::Interpolator>& interpolator : interpolators_)
                {
                    interpolator->Update(t, particle);
                }
            }
        }
    }
}
