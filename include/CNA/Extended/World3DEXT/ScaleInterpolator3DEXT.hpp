// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ScaleInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Interpolators::ScaleInterpolator,
// same shape (Phase 11 C-4, 2026-07-14) -- meaningful now that Particle3DEXT::ScaleEXT is a
// Vector2 (Phase 11 C-1's ScaleEXT float->Vector2 change), matching 2D's own Vector2 scale.
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates a particle's ScaleEXT between StartValue and EndValue over its lifetime. */
    class ScaleInterpolator3DEXT final : public InterpolatorOfT3DEXT<Microsoft::Xna::Framework::Vector2>
    {
    public:
        ScaleInterpolator3DEXT()
            : InterpolatorOfT3DEXT("ScaleInterpolator3DEXT", Microsoft::Xna::Framework::Vector2(1.0f, 1.0f),
                                    Microsoft::Xna::Framework::Vector2(1.0f, 1.0f))
        {
        }

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
