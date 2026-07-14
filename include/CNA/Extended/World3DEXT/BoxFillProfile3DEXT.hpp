// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::BoxFillProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::BoxFillProfile, same shape extended
// one dimension further (Phase 11 C-2, 2026-07-14): emits particles from anywhere within a
// box volume (not just its faces -- see BoxProfile3DEXT.hpp for that).
#pragma once

#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles from anywhere within a box volume (centered at the origin), random headings. */
    class BoxFillProfile3DEXT final : public Profile3DEXT
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
