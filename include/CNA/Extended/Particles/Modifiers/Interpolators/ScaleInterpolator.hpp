// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Interpolators/ScaleInterpolator.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Interpolators/InterpolatorOfT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Particles::Modifiers::Interpolators
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Interpolates a particle's scale between StartValue and EndValue over its lifetime. */
    class ScaleInterpolator : public InterpolatorOfT<Vector2>
    {
    public:
        ScaleInterpolator() : InterpolatorOfT("ScaleInterpolator") {}

        void Update(float amount, Data::Particle* particle) override;
    };
}
