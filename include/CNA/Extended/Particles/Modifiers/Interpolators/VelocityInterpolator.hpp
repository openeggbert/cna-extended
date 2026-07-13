// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/VelocityInterpolator.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/InterpolatorOfT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Interpolates a particle's velocity between StartValue and EndValue over its lifetime. */
    class VelocityInterpolator : public InterpolatorOfT<Vector2>
    {
    public:
        VelocityInterpolator() : InterpolatorOfT("VelocityInterpolator") {}

        void Update(float amount, Data::Particle* particle) override;
    };
}
