// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Containers/RectangleLoopContainerModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers::Containers
{
    /** @brief Wraps particles to the opposite edge when they cross a rectangle centered at each particle's trigger position. */
    class RectangleLoopContainerModifier : public Modifier
    {
    public:
        int Width = 0;
        int Height = 0;

        RectangleLoopContainerModifier() : Modifier("RectangleLoopContainerModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
