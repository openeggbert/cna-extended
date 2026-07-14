// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CircleProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::CircleProfile (Phase 11 C-2,
// 2026-07-14): the sphere-fill analog of 2D's filled-disk emission (see RingProfile3DEXT.hpp
// for the same "genuinely 3D, not embedded 2D shape" rationale). Reproduces 2D CircleProfile's
// exact linear (not volume-uniform) radial distance sampling -- offset distance is drawn
// uniformly in [0, RadiusEXT], which is not actually a uniform fill (samples cluster slightly
// toward the center); this is upstream's own established behavior for the 2D profile this
// generalizes, not "fixed" here since a real per-particle-count-visible bias correction is a
// separate, deliberate feature, not a bug in what this class is documented to do.
#pragma once

#include "CNA/Extended/World3DEXT/CircleRadiation3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles from anywhere within a spherical volume, with a configurable radiation pattern for headings. */
    class CircleProfile3DEXT final : public Profile3DEXT
    {
    public:
        float RadiusEXT = 0.0f;
        CircleRadiation3DEXT RadiateEXT = CircleRadiation3DEXT::None;

        /** @throws System::ArgumentOutOfRangeException RadiateEXT is not a recognized value. */
        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
