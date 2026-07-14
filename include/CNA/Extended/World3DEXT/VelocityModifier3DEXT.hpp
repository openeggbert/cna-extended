// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::VelocityModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::VelocityModifier, same shape (Phase
// 11 C-3, 2026-07-14): drives owned Interpolator3DEXT instances by particle speed relative
// to VelocityThresholdEXT, rather than by age (see AgeModifier3DEXT for the age-driven
// counterpart -- both own the same Interpolator3DEXT list type).
#pragma once

#include "CNA/Extended/World3DEXT/Interpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies a collection of Interpolator3DEXT instances to particles, with intensity based on velocity magnitude relative to VelocityThresholdEXT. */
    class VelocityModifier3DEXT final : public Modifier3DEXT
    {
    public:
        float VelocityThresholdEXT = 0.0f;

        VelocityModifier3DEXT() : Modifier3DEXT("VelocityModifier3DEXT") {}

        /** @brief Gets the mutable list of interpolators applied to particles. */
        [[nodiscard]] std::vector<std::unique_ptr<Interpolator3DEXT>>& getInterpolatorsProperty() { return interpolators_; }

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;

    private:
        std::vector<std::unique_ptr<Interpolator3DEXT>> interpolators_;
    };
}
