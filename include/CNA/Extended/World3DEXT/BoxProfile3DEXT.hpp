// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BoxProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::BoxProfile (Phase 11 C-2,
// 2026-07-14): per the approved plan, a genuinely 3D box's *six faces* (not 2D's four
// edges), chosen with equal probability, matching 2D BoxProfile's own "equal probability per
// side" semantics extended one dimension further.
#pragma once

#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles from one of the six faces of a box (equal probability per face, centered at the origin), random headings. */
    class BoxProfile3DEXT final : public Profile3DEXT
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
