// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::OpacityFastFadeModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::OpacityFastFadeModifier, same shape
// (Phase 11 C-3, 2026-07-14). Assumes a standard 1.0-second particle lifespan (sets
// OpacityEXT = 1 - AgeEXT directly, not accumulated); a documented upstream quirk, not
// "fixed" here (matching 2D's own header comment for the same reasoning).
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Sets each particle's OpacityEXT to (1 - AgeEXT) every update, fading it out linearly over a 1-second lifespan. */
    class OpacityFastFadeModifier3DEXT final : public Modifier3DEXT
    {
    public:
        OpacityFastFadeModifier3DEXT() : Modifier3DEXT("OpacityFastFadeModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
