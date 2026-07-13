// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Interpolators/ScaleInterpolator.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    void ScaleInterpolator::Update(float amount, Data::Particle* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->Scale[0] = StartValue.X + (EndValue.X - StartValue.X) * amount;
        particle->Scale[1] = StartValue.Y + (EndValue.Y - StartValue.Y) * amount;
    }
}
