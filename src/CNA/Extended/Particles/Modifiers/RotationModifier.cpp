// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/RotationModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    void RotationModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const float rotationRateDelta = RotationRate * elapsedSeconds;

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            particle->Rotation += rotationRateDelta;
        }
    }
}
