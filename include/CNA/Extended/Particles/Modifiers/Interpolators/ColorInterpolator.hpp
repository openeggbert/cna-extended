// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/ColorInterpolator.cs.
#pragma once

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/Particles/Modifiers/Interpolators/InterpolatorOfT.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    /** @brief Interpolates a particle's HSL color between StartValue and EndValue over its lifetime. */
    class ColorInterpolator final : public InterpolatorOfT<HslColor>
    {
    public:
        ColorInterpolator() : InterpolatorOfT("ColorInterpolator") {}

        void Update(float amount, Data::Particle* particle) override;
    };
}
