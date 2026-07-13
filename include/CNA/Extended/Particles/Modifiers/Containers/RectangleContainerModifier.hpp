// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Containers/RectangleContainerModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers::Containers
{
    /** @brief Constrains particles inside a rectangle centered at each particle's trigger position, reflecting off the edges. */
    class RectangleContainerModifier final : public Modifier
    {
    public:
        int Width = 0;
        int Height = 0;
        float RestitutionCoefficient = 1.0f;

        RectangleContainerModifier() : Modifier("RectangleContainerModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
