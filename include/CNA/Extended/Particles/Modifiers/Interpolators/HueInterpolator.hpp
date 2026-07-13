// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/HueInterpolator.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/InterpolatorOfT.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    /** @brief Interpolates only a particle's hue (Color[0]) between StartValue and EndValue, preserving saturation/lightness. */
    class HueInterpolator : public InterpolatorOfT<float>
    {
    public:
        HueInterpolator() : InterpolatorOfT("HueInterpolator") {}

        void Update(float amount, Data::Particle* particle) override;
    };
}
