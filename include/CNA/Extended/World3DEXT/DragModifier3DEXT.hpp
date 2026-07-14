// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::DragModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::DragModifier, same shape extended
// to a 3-axis velocity (Phase 11 C-3, 2026-07-14).
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies fluid-resistance drag to particles, damping VelocityEXT proportional to VelocityEXT * MassEXT * DragCoefficientEXT * DensityEXT. */
    class DragModifier3DEXT final : public Modifier3DEXT
    {
    public:
        float DragCoefficientEXT = 0.47f;
        float DensityEXT = 0.5f;

        DragModifier3DEXT() : Modifier3DEXT("DragModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
