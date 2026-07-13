// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/VelocityColorModifier.cs.
#pragma once

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Colors particles between StationaryColor and VelocityColor based on their speed relative to VelocityThreshold. */
    class VelocityColorModifier : public Modifier
    {
    public:
        HslColor StationaryColor;
        HslColor VelocityColor;
        float VelocityThreshold = 0.0f;

        VelocityColorModifier() : Modifier("VelocityColorModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
