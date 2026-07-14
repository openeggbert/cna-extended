// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::VelocityInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::Interpolators::
// VelocityInterpolator, which uses Vector2 and only ever touches Particle::Velocity's X/Y
// components. Per the "genuinely 3D, not a 2D vector embedded in 3D" principle established
// throughout Phase 11 C-2/C-3 (e.g. VelocityInterpolator3DEXT's own sibling
// VortexModifier3DEXT), this interpolates the full Vector3 Particle3DEXT::VelocityEXT
// (X/Y/Z), not just two of its three components.
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates a particle's VelocityEXT between StartValue and EndValue over its lifetime. */
    class VelocityInterpolator3DEXT final : public InterpolatorOfT3DEXT<Microsoft::Xna::Framework::Vector3>
    {
    public:
        VelocityInterpolator3DEXT()
            : InterpolatorOfT3DEXT("VelocityInterpolator3DEXT", Microsoft::Xna::Framework::Vector3::Zero,
                                    Microsoft::Xna::Framework::Vector3::Zero)
        {
        }

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
