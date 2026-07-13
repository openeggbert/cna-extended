// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/LinearGravityModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Particles::Modifiers
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Applies a constant directional force (proportional to each particle's mass), simulating gravity or wind. */
    class LinearGravityModifier : public Modifier
    {
    public:
        Vector2 Direction;
        float Strength = 0.0f;

        LinearGravityModifier() : Modifier("LinearGravityModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;
    };
}
