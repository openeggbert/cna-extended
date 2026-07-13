// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/OpacityFastFadeModifier.cs. Assumes a
// standard 1.0-second particle lifespan (sets Opacity = 1 - Age directly, not accumulated);
// documented upstream quirk, not "fixed" here.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    /** @brief Sets each particle's opacity to (1 - Age) every update, fading it out linearly over a 1-second lifespan. */
    class OpacityFastFadeModifier final : public Modifier
    {
    public:
        OpacityFastFadeModifier() : Modifier("OpacityFastFadeModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
