// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::LinearGravityModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// Not a literal 1:1 port of CNA::Extended::Particles::Modifiers::LinearGravityModifier,
// which splits gravity into a Direction (unit vector) and a Strength (magnitude); this class
// keeps ParticleEmitter3DEXT's own existing single-Vector3 GravityEXT model (a raw
// acceleration, not force/mass) instead, matching what ParticleEmitter3DEXT's pre-existing
// baked gravity logic already did before this architecture existed -- introducing a
// Direction/Strength split would be a real API change with no corresponding 3D need
// identified yet, not a pure refactor. particle->MassEXT is still applied (defaulting to
// 1.0, a no-op for every particle emitted today), matching 2D's own mass-weighted gravity
// and keeping this class forward-compatible if per-particle mass ranges are ever added.
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Applies a constant acceleration (GravityEXT) to every touched particle's velocity, scaled by its MassEXT. */
    class LinearGravityModifier3DEXT final : public Modifier3DEXT
    {
    public:
        /** @brief Constant acceleration (units/second^2) applied to touched particles' velocity each Update(). */
        Microsoft::Xna::Framework::Vector3 GravityEXT = Microsoft::Xna::Framework::Vector3::Zero;

        LinearGravityModifier3DEXT() : Modifier3DEXT("LinearGravityModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;
    };
}
