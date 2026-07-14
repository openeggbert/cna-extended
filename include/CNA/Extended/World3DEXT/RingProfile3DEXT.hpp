// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::RingProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::RingProfile (Phase 11 C-2,
// 2026-07-14): NOT a literal geometric ring/torus -- per the approved plan, this is the
// sphere-surface analog of 2D's circle-perimeter emission (matching PointProfile3DEXT's own
// precedent of generalizing a 2D circle concept to a 3D sphere), the "genuinely 3D, not a 2D
// shape embedded in 3D" spirit established throughout World3DEXT.
#pragma once

#include "CNA/Extended/World3DEXT/CircleRadiation3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles from the surface of a sphere, with a configurable radiation pattern for headings. */
    class RingProfile3DEXT final : public Profile3DEXT
    {
    public:
        float RadiusEXT = 0.0f;
        CircleRadiation3DEXT RadiateEXT = CircleRadiation3DEXT::None;

        /** @throws System::ArgumentOutOfRangeException RadiateEXT is not a recognized value. */
        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
