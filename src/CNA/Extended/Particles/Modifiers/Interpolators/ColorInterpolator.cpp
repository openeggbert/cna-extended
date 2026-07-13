// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Interpolators/ColorInterpolator.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    void ColorInterpolator::Update(float amount, Data::Particle* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const float h = StartValue.getHProperty() + (EndValue.getHProperty() - StartValue.getHProperty()) * amount;
        const float s = StartValue.getSProperty() + (EndValue.getSProperty() - StartValue.getSProperty()) * amount;
        const float l = StartValue.getLProperty() + (EndValue.getLProperty() - StartValue.getLProperty()) * amount;

        particle->Color[0] = h;
        particle->Color[1] = s;
        particle->Color[2] = l;
    }
}
