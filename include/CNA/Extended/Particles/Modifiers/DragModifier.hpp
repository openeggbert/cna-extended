// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/DragModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Applies fluid-resistance drag to particles, damping their velocity proportional to velocity * mass * DragCoefficient * Density. */
    class DragModifier : public Modifier
    {
    public:
        float DragCoefficient = 0.47f;
        float Density = 0.5f;

        DragModifier() : Modifier("DragModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
