// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/OpacityInterpolator.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/InterpolatorOfT.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    /** @brief Interpolates a particle's opacity between StartValue and EndValue over its lifetime. */
    class OpacityInterpolator : public InterpolatorOfT<float>
    {
    public:
        OpacityInterpolator() : InterpolatorOfT("OpacityInterpolator") {}

        void Update(float amount, Data::Particle* particle) override;
    };
}
