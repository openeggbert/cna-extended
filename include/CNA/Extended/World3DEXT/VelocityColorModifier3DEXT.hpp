// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::VelocityColorModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::VelocityColorModifier,
// which interpolates in HSL space (Particle::Color is HSL in 2D). Particle3DEXT::ColorEXT is
// a plain RGB Microsoft::Xna::Framework::Color, matching ColorInterpolator3DEXT's own
// precedent (see that class's header comment) of doing a direct per-channel RGB lerp instead
// of introducing HSL interpolation.
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Colors particles between StationaryColorEXT and VelocityColorEXT based on their speed relative to VelocityThresholdEXT. */
    class VelocityColorModifier3DEXT final : public Modifier3DEXT
    {
    public:
        Microsoft::Xna::Framework::Color StationaryColorEXT = Microsoft::Xna::Framework::Color::White;
        Microsoft::Xna::Framework::Color VelocityColorEXT = Microsoft::Xna::Framework::Color::White;
        float VelocityThresholdEXT = 0.0f;

        VelocityColorModifier3DEXT() : Modifier3DEXT("VelocityColorModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
