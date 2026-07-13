// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/ParticleReleaseParameters.cs. Upstream's mutable
// public fields with default-value initializers -> plain public mutable fields here too (matching
// the "public struct with mutable fields" convention already used for the sibling Particle*Parameter
// types, since these fields are meant to be freely get/set by consumers configuring an emitter, not
// encapsulated behind accessors).
#pragma once

#include "CNA/Extended/Particles/Data/ParticleColorParameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleFloatParameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleInt32Parameter.hpp"
#include "CNA/Extended/Particles/Data/ParticleVector2Parameter.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::Particles::Data
{
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;

    /** @brief The configurable properties controlling how particles are initialized when released from an emitter. */
    class ParticleReleaseParameters
    {
    public:
        /** @brief Number of particles released per emission. Defaults to random 5-100. */
        ParticleInt32Parameter Quantity{5, 100};

        /** @brief Initial speed of released particles. Defaults to random 50.0-100.0. */
        ParticleFloatParameter Speed{50.0f, 100.0f};

        /** @brief Initial color of released particles. Defaults to constant white (1,1,1). */
        ParticleColorParameter Color{Vector3(1.0f, 1.0f, 1.0f)};

        /** @brief Initial opacity of released particles. Defaults to random 0.0-1.0. */
        ParticleFloatParameter Opacity{0.0f, 1.0f};

        /** @brief Initial scale of released particles. Defaults to random (0.5,0.5)-(1.0,1.0). */
        ParticleVector2Parameter Scale{Vector2(0.5f, 0.5f), Vector2(1.0f, 1.0f)};

        /** @brief Initial rotation (radians) of released particles. Defaults to random -pi to pi. */
        ParticleFloatParameter Rotation{-MathHelper::Pi, MathHelper::Pi};

        /** @brief Mass of released particles. Defaults to constant 1.0. */
        ParticleFloatParameter Mass{1.0f};

        ParticleReleaseParameters() = default;
    };
}
