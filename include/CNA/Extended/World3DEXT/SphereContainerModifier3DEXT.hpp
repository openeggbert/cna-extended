// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::SphereContainerModifier3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Modifiers::Containers::CircleContainerModifier
// (Phase 11 C-3, 2026-07-14) -- a sphere (not a circle) centered at each particle's
// TriggeredPositionEXT. Stays flat in CNA::Extended::World3DEXT rather than mirroring 2D's
// Containers sub-namespace, matching every other Phase 11 C-1/C-2/C-3 type (see
// ParticleEmitter3DEXT.hpp's header comment for the "stay flat" precedent).
#pragma once

#include "CNA/Extended/World3DEXT/Modifier3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Constrains particles inside (or outside) a sphere centered at each particle's trigger position, reflecting at the boundary. */
    class SphereContainerModifier3DEXT final : public Modifier3DEXT
    {
    public:
        float RadiusEXT = 0.0f;
        bool InsideEXT = true;
        float RestitutionCoefficientEXT = 1.0f;

        SphereContainerModifier3DEXT() : Modifier3DEXT("SphereContainerModifier3DEXT") {}

    protected:
        void Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount) override;

    private:
        void SetReflectedEXT(float distSq, Particle3DEXT& particle, const Microsoft::Xna::Framework::Vector3& normal);
    };
}
