// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/LineRadiation.cs.
#pragma once

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Radiation pattern for LineProfile: how a particle's initial heading is determined relative to the line. */
    enum class LineRadiation
    {
        /** @brief Particles move in random directions unrelated to their position. */
        None,
        /** @brief All particles are given the same heading, LineProfile::Direction. */
        Directional,
        /** @brief Particles move perpendicular to the line axis, upward in screen coordinates. */
        PerpendicularUp,
        /** @brief Particles move perpendicular to the line axis, downward in screen coordinates. */
        PerpendicularDown
    };
}
