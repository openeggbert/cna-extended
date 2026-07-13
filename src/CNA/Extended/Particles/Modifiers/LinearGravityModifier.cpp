// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/LinearGravityModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    void LinearGravityModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const Vector2 vector = Direction * (Strength * elapsedSeconds);

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            particle->Velocity[0] = particle->Velocity[0] + vector.X * particle->Mass;
            particle->Velocity[1] = particle->Velocity[1] + vector.Y * particle->Mass;
        }
    }
}
