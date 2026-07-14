// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RotationModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::RotationModifier, same shape (Phase
// 11 C-3, 2026-07-14): applies a constant rotational velocity to Particle3DEXT::RotationEXT
// (the particle's billboard roll rotation), not a full 3D angular-velocity/orientation model
// -- matching RotationEXT's own existing single-scalar-radians shape (see Particle3DEXT.hpp).
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies a constant rotational velocity (RotationRateEXT radians/second) to particles' RotationEXT. */
    class RotationModifier3DEXT final : public Modifier3DEXT
    {
    public:
        float RotationRateEXT = 0.0f;

        RotationModifier3DEXT() : Modifier3DEXT("RotationModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
