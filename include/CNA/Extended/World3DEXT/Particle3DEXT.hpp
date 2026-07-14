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
//
// RotationEXT/MassEXT/TriggeredPositionEXT and ScaleEXT's float->Vector2 change (2026-07-14,
// user-confirmed) were added to bring the Profiles/Modifiers/Interpolators plugin
// architecture (Profile3DEXT/Modifier3DEXT/Interpolator3DEXT) to parity with 2D's own
// Data::Particle fields -- RotationModifier3DEXT/RotationInterpolator3DEXT need RotationEXT,
// DragModifier3DEXT/mass-weighted gravity need MassEXT, the 3D Container modifiers need
// TriggeredPositionEXT, and ScaleInterpolator3DEXT needs a non-uniform (Vector2) scale to be
// meaningful at all -- none of these existed when ParticleEmitter3DEXT only had cone
// emission + gravity/expiry/color-opacity interpolation baked directly into UpdateEXT.
// ScaleEXT's new default (Vector2(1,1)) preserves existing uniform-scale visuals unchanged
// (ParticleRenderSystem3DEXT.cpp's single Vector2(scale,scale) call site now reads
// particle.ScaleEXT directly instead of constructing that Vector2 itself).
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
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

        /** @brief The current billboard scale (world-space quad width/height) of this particle. */
        Microsoft::Xna::Framework::Vector2 ScaleEXT = Microsoft::Xna::Framework::Vector2(1.0f, 1.0f);

        /** @brief The current opacity of this particle, in [0, 1]. */
        float OpacityEXT = 1.0f;

        /** @brief The current billboard roll rotation of this particle, in radians. */
        float RotationEXT = 0.0f;

        /** @brief The mass of this particle, used by mass-weighted modifiers (e.g. gravity). */
        float MassEXT = 1.0f;

        /** @brief The world-space position this particle was released from (its emitter's origin at release time, not its current position). */
        Microsoft::Xna::Framework::Vector3 TriggeredPositionEXT;

        /** @brief Gets AgeEXT / LifetimeEXT, clamped to [0, 1]. Used to interpolate color/opacity over the particle's life. */
        [[nodiscard]] float GetAgeRatioEXT() const;

        /** @brief Gets whether this particle has reached or exceeded its LifetimeEXT. */
        [[nodiscard]] bool IsExpiredEXT() const { return AgeEXT >= LifetimeEXT; }
    };
}
