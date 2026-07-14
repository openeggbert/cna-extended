// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::CircleRadiation3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::CircleRadiation, same shape (Phase
// 11 C-2, 2026-07-14) -- shared by RingProfile3DEXT (sphere-surface emission, the 3D analog
// of a circle's perimeter) and CircleProfile3DEXT (sphere-fill emission, the 3D analog of a
// filled disk), matching 2D's own sharing of CircleRadiation between RingProfile/
// CircleProfile.
#pragma once

namespace CNA::Extended::World3DEXT
{
    /** @brief Radiation pattern for RingProfile3DEXT/CircleProfile3DEXT: how a particle's position relates to its initial heading. */
    enum class CircleRadiation3DEXT
    {
        /** @brief Particles move in a uniformly random direction, unrelated to their position. */
        None,
        /** @brief Particles move toward the center. */
        In,
        /** @brief Particles move away from the center. */
        Out
    };
}
