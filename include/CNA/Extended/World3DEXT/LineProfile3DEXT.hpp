// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::LineProfile3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::LineProfile (Phase 11 C-2,
// 2026-07-14): emits particles uniformly along a 3D line segment. See LineRadiation3DEXT.hpp
// for the one deliberate deviation from the 2D 1:1 mirror (PerpendicularUp/Down collapsed
// into a single Perpendicular mode).
#pragma once

#include "CNA/Extended/World3DEXT/LineRadiation3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief Emits particles uniformly along a 3D line segment, with a configurable radiation pattern for headings. */
    class LineProfile3DEXT final : public Profile3DEXT
    {
    public:
        /** @brief Axis the line segment runs along, in world space. Normalized internally. */
        Microsoft::Xna::Framework::Vector3 AxisEXT = Microsoft::Xna::Framework::Vector3::Up;

        /** @brief Total length of the line segment; particles are offset in [-LengthEXT/2, LengthEXT/2] along AxisEXT. */
        float LengthEXT = 0.0f;

        /** @brief Heading for Directional mode, ignored for None/Perpendicular. Normalized internally. */
        Microsoft::Xna::Framework::Vector3 DirectionEXT = Microsoft::Xna::Framework::Vector3::Up;

        LineRadiation3DEXT RadiateEXT = LineRadiation3DEXT::None;

        /** @throws System::InvalidOperationException RadiateEXT is not a recognized value. */
        void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                  System::Random& random) override;
    };
}
