// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::LineRadiation3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::LineRadiation (Phase 11 C-2,
// 2026-07-14). Not a literal 1:1 mirror: 2D's PerpendicularUp/PerpendicularDown pair is
// collapsed into a single Perpendicular value here -- a 3D line's perpendicular directions
// form an entire plane, not two fixed screen-space "up"/"down" choices (which only make
// sense for a 2D line, where there is exactly one perpendicular direction up to sign). There
// is no non-arbitrary way to split 3D's perpendicular plane into two named modes, so a
// single uniformly-random-direction-within-that-plane mode is the natural 3D generalization
// (see LineProfile3DEXT.cpp).
#pragma once

namespace CNA::Extended::World3DEXT
{
    /** @brief Radiation pattern for LineProfile3DEXT: how a particle's initial heading is determined relative to the line. */
    enum class LineRadiation3DEXT
    {
        /** @brief Particles move in a uniformly random direction over the full sphere, unrelated to their position. */
        None,
        /** @brief All particles are given the same heading, LineProfile3DEXT::DirectionEXT. */
        Directional,
        /** @brief Particles move in a uniformly random direction within the plane perpendicular to LineProfile3DEXT::AxisEXT. */
        Perpendicular
    };
}
