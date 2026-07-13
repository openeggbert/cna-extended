// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Interpolators/HueInterpolator.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    void HueInterpolator::Update(float amount, Data::Particle* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->Color[0] = StartValue + (EndValue - StartValue) * amount;
    }
}
