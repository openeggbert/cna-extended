// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Modifiers/Containers/CircleContainerModifier.cs.
#pragma once

#include "CNA/Extended/Particles/Modifiers/Modifier.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended::Particles::Data
{
    struct Particle;
}

namespace CNA::Extended::Particles::Modifiers::Containers
{
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Constrains particles inside (or outside) a circle centered at each particle's trigger position, reflecting at the boundary. */
    class CircleContainerModifier : public Modifier
    {
    public:
        float Radius = 0.0f;
        bool Inside = true;
        float RestitutionCoefficient = 1.0f;

        CircleContainerModifier() : Modifier("CircleContainerModifier") {}

    protected:
        void Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount) override;

    private:
        void SetReflected(float distSq, Data::Particle* particle, const Vector2& normal);
    };
}
