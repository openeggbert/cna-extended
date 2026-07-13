// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleRenderingOrder.cs.
#pragma once

namespace CNA::Extended::Particles
{
    /** @brief Specifies whether particles are rendered front-to-back or back-to-front. */
    enum class ParticleRenderingOrder
    {
        FrontToBack,
        BackToFront
    };
}
