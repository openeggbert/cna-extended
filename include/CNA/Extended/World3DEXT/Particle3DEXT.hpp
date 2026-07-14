// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Particle3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Data::Particle, changing 2D packed-float-array
// position/velocity/color/scale fields to Vector3, per plan3d.md's Phase 7 scope. A plain
// value type held in a std::vector<Particle3DEXT> by ParticleEmitter3DEXT, not upstream's
// #pragma pack(1)/raw-pointer-walked ParticleBuffer -- that tight packing existed to match a
// C# `[StructLayout(Pack=1)] unsafe struct`'s exact in-memory layout, which has no
// counterpart here (this is new code, not a byte-compatible port of anything), so a plain
// std::vector is simpler and equally correct.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    /** @brief An individual 3D particle, owned by a ParticleEmitter3DEXT. */
    struct Particle3DEXT
    {
        /** @brief The current age (in seconds) of this particle. */
        float AgeEXT = 0.0f;

        /** @brief The total lifetime (in seconds) this particle expires at. */
        float LifetimeEXT = 1.0f;

        /** @brief The current world-space position of this particle. */
        Microsoft::Xna::Framework::Vector3 PositionEXT;

        /** @brief The current world-space velocity of this particle, in units/second. */
        Microsoft::Xna::Framework::Vector3 VelocityEXT;

        /** @brief The current color of this particle. */
        Microsoft::Xna::Framework::Color ColorEXT = Microsoft::Xna::Framework::Color::White;

        /** @brief The current billboard scale (world-space quad size) of this particle. */
        float ScaleEXT = 1.0f;

        /** @brief The current opacity of this particle, in [0, 1]. */
        float OpacityEXT = 1.0f;

        /** @brief Gets AgeEXT / LifetimeEXT, clamped to [0, 1]. Used to interpolate color/opacity over the particle's life. */
        [[nodiscard]] float GetAgeRatioEXT() const;

        /** @brief Gets whether this particle has reached or exceeded its LifetimeEXT. */
        [[nodiscard]] bool IsExpiredEXT() const { return AgeEXT >= LifetimeEXT; }
    };
}
