// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Interpolators/OpacityInterpolator.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    void OpacityInterpolator::Update(float amount, Data::Particle* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->Opacity = StartValue + (EndValue - StartValue) * amount;
    }
}
