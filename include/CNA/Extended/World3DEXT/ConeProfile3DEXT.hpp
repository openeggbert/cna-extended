// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ConeProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// NOT a 2D Profile port -- no 2D emission Profile maps cleanly onto a 3D solid cone
// distribution (2D's own SprayProfile is a flat 2D-arc spray around a 2D Direction, not a
// solid-angle cone; a literal port would not reproduce this project's own established 3D
// cone semantics). Instead, this wraps the exact cone-sampling algorithm
// ParticleEmitter3DEXT used directly (as a private SampleConeDirectionEXT method) before the
// Profile3DEXT/Modifier3DEXT/Interpolator3DEXT architecture existed -- DirectionEXT/
// HalfAngleEXT are the same fields, same defaults (Vector3::Up / pi -- pi degenerates to
// full-sphere emission, the common "explosion"/"ambient" case), same math, just moved from
// ParticleEmitter3DEXT's own public fields onto this profile object, matching where 2D's own
// analogous per-shape configuration (e.g. SprayProfile::Direction/Spread) actually lives.
// User-confirmed design decision (2026-07-14): existing tests that configured
// emitter.ConeDirectionEXT/ConeHalfAngleEXT directly were updated to configure this profile
// instead, via ParticleEmitter3DEXT::getProfileEXTProperty().
#pragma once

#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits all particles from a single point in a directional solid-angle cone (DirectionEXT +/- HalfAngleEXT). */
    class ConeProfile3DEXT final : public Profile3DEXT
    {
    public:
        /** @brief Direction the emission cone points, in world space. Normalized internally. */
        Microsoft::Xna::Framework::Vector3 DirectionEXT = Microsoft::Xna::Framework::Vector3::Up;

        /** @brief Half-angle (radians) of the emission cone around DirectionEXT. MathHelper::Pi (the default) emits uniformly over the full sphere. */
        float HalfAngleEXT = 3.14159265358979323846f;

        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
