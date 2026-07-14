// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ColorInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::Interpolators::
// ColorInterpolator, which interpolates in HSL space (Particle::Color is stored as HSL,
// converted to RGB only at render time). Particle3DEXT::ColorEXT is a plain
// Microsoft::Xna::Framework::Color (RGB+A), and ParticleEmitter3DEXT's own pre-existing
// baked color-interpolation logic (before this architecture existed) already did a direct
// per-channel RGB lerp, not HSL -- this class reproduces that exact RGB lerp (including
// always emitting alpha=255, unchanged/not interpolated, matching the original) rather than
// introducing HSL interpolation, which would be a real, visible color-path change, not a
// pure refactor.
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates a particle's RGB color between StartValue and EndValue over its lifetime (alpha always emitted as 255). */
    class ColorInterpolator3DEXT final : public InterpolatorOfT3DEXT<Microsoft::Xna::Framework::Color>
    {
    public:
        ColorInterpolator3DEXT()
            : InterpolatorOfT3DEXT("ColorInterpolator3DEXT", Microsoft::Xna::Framework::Color::White, Microsoft::Xna::Framework::Color::White)
        {
        }

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
