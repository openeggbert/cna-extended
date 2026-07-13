// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleValueKind.cs.
#pragma once

namespace CNA::Extended::Particles::Data
{
    /** @brief Defines how particle property values are determined during particle creation and simulation. */
    enum class ParticleValueKind
    {
        /** @brief The property maintains a constant value across all particles. */
        Constant,

        /** @brief Each particle receives a unique random value within a defined min/max range. */
        Random
    };
}
