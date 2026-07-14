// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RotationInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Interpolators::RotationInterpolator,
// same shape (Phase 11 C-4, 2026-07-14): interpolates Particle3DEXT::RotationEXT (the
// particle's billboard roll rotation, see RotationModifier3DEXT.hpp).
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates a particle's RotationEXT between StartValue and EndValue over its lifetime. */
    class RotationInterpolator3DEXT final : public InterpolatorOfT3DEXT<float>
    {
    public:
        RotationInterpolator3DEXT() : InterpolatorOfT3DEXT("RotationInterpolator3DEXT", 0.0f, 0.0f) {}

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
