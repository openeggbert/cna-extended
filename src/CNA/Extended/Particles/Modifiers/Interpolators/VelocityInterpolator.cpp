// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Interpolators/VelocityInterpolator.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    void VelocityInterpolator::Update(float amount, Data::Particle* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->Velocity[0] = StartValue.X + (EndValue.X - StartValue.X) * amount;
        particle->Velocity[1] = StartValue.Y + (EndValue.Y - StartValue.Y) * amount;
    }
}
