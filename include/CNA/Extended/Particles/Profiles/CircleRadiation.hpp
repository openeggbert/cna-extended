// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Profiles/CircleRadiation.cs.
#pragma once

namespace CNA::Extended::Particles::Profiles
{
    /** @brief Radiation pattern for CircleProfile/RingProfile: how a particle's position relates to its initial heading. */
    enum class CircleRadiation
    {
        /** @brief Particles move in random directions unrelated to their position. */
        None,
        /** @brief Particles move toward the center. */
        In,
        /** @brief Particles move away from the center. */
        Out
    };
}
