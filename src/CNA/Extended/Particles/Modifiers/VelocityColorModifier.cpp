// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/VelocityColorModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

#include <cmath>

namespace CNA::Extended::Particles::Modifiers
{
    void VelocityColorModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
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
                particle->Color[0] = VelocityColor.getHProperty();
                particle->Color[1] = VelocityColor.getSProperty();
                particle->Color[2] = VelocityColor.getLProperty();
            }
            else
            {
                const HslColor deltaColor = VelocityColor - StationaryColor;
                const float t = std::sqrt(velocitySquared) / VelocityThreshold;

                const float h = deltaColor.getHProperty() * t + StationaryColor.getHProperty();
                const float s = deltaColor.getSProperty() * t + StationaryColor.getSProperty();
                const float l = deltaColor.getLProperty() * t + StationaryColor.getLProperty();

                particle->Color[0] = h;
                particle->Color[1] = s;
                particle->Color[2] = l;
            }
        }
    }
}
