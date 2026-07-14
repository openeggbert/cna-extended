// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BoxUniformProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::BoxUniformProfile (Phase 11 C-2,
// 2026-07-14): emits from the box's surface (its six faces, see BoxProfile3DEXT.hpp),
// weighted by each face's *area* -- the natural 3D generalization of 2D BoxUniformProfile's
// per-edge-*length* weighting (a box's boundary measure is surface area, not perimeter).
// One deliberate deviation from a literal port: face selection here draws a continuous
// float in [0, totalSurfaceArea) rather than 2D's `Next(static_cast<int>(perimeter))`
// (an integer cast that truncates the fractional part of Width/Height). Since this is
// brand-new 3D code with no pre-existing behavior to preserve, the continuous version is
// used instead of carrying that truncation artifact forward.
#pragma once

#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles from the surface of a box (centered at the origin), each face weighted by its area, random headings. */
    class BoxUniformProfile3DEXT final : public Profile3DEXT
    {
    public:
        /** @brief Extent of the box along the X axis. */
        float WidthEXT = 0.0f;
        /** @brief Extent of the box along the Y axis. */
        float HeightEXT = 0.0f;
        /** @brief Extent of the box along the Z axis. */
        float DepthEXT = 0.0f;

        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
