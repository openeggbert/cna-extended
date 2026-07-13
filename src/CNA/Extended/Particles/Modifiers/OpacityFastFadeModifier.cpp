// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/OpacityFastFadeModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    void OpacityFastFadeModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            particle->Opacity = 1.0f - particle->Age;
        }
    }
}
