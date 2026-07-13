// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/DragModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    void DragModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            const float drag = -DragCoefficient * Density * particle->Mass * elapsedSeconds;

            particle->Velocity[0] = particle->Velocity[0] + particle->Velocity[0] * drag;
            particle->Velocity[1] = particle->Velocity[1] + particle->Velocity[1] * drag;
        }
    }
}
