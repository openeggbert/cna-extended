// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::AgeModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::AgeModifier, same shape
// (2026-07-14, user-requested parity work): drives every owned Interpolator3DEXT from each
// touched particle's age fraction. Like its 2D counterpart, this modifier does not itself
// advance AgeEXT -- ParticleEmitter3DEXT::UpdateEXT does that directly for every particle,
// every call (matching 2D's own ParticleEmitter::Update, which likewise bakes basic
// age/position integration in directly rather than driving it through a Modifier -- see
// ParticleEmitter3DEXT.hpp's header comment).
#pragma once

#include "CNA/Extended/World3DEXT/Interpolator3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies a collection of Interpolator3DEXT instances to each particle, driven by the particle's age (as a fraction of its lifetime). */
    class AgeModifier3DEXT final : public Modifier3DEXT
    {
    public:
        AgeModifier3DEXT() : Modifier3DEXT("AgeModifier3DEXT") {}

        /** @brief Gets the mutable list of interpolators applied to particles. */
        [[nodiscard]] std::vector<std::unique_ptr<Interpolator3DEXT>>& getInterpolatorsProperty() { return interpolators_; }

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;

    private:
        std::vector<std::unique_ptr<Interpolator3DEXT>> interpolators_;
    };
}
