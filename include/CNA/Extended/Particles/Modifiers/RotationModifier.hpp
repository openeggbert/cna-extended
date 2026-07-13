// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/RotationModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Applies a constant rotational velocity (RotationRate radians/second) to particles. */
    class RotationModifier : public Modifier
    {
    public:
        float RotationRate = 0.0f;

        RotationModifier() : Modifier("RotationModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
