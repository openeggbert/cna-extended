// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::OpacityInterpolator3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Interpolators::OpacityInterpolator,
// same shape (2026-07-14, user-requested parity work).
#pragma once

#include "CNA/Extended/World3DEXT/InterpolatorOfT3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Interpolates a particle's opacity between StartValue and EndValue over its lifetime. */
    class OpacityInterpolator3DEXT final : public InterpolatorOfT3DEXT<float>
    {
    public:
        OpacityInterpolator3DEXT() : InterpolatorOfT3DEXT("OpacityInterpolator3DEXT", 1.0f, 0.0f) {}

        void Update(float amount, Particle3DEXT* particle) override;
    };
}
