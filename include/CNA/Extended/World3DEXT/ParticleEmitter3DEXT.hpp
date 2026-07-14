// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleEmitter3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::ParticleEmitter. Originally deliberately
// scoped down (one built-in cone emission shape, three behaviors baked directly into
// UpdateEXT); brought to parity with 2D's full Profiles/Modifiers/Interpolators plugin
// architecture (2026-07-14, user-requested) via Profile3DEXT/Modifier3DEXT/
// Interpolator3DEXT (see those files' own header comments for the full design, including
// Modifier3DEXT's documented deviation in how Frequency-throttling interacts with elapsed
// time).
//
// Design note on which fields moved and which didn't (deliberately minimal blast radius on
// this class's own public surface, not a blanket "everything moves to a plugin object"
// rule): ConeDirectionEXT/ConeHalfAngleEXT moved to ConeProfile3DEXT (getProfileEXTProperty())
// -- a user-confirmed decision specifically about these two fields, since 2D's own analogous
// per-shape configuration (e.g. SprayProfile::Direction/Spread) genuinely lives on the
// Profile object, not the emitter. StartColorEXT/EndColorEXT/StartOpacityEXT/EndOpacityEXT/
// GravityEXT stay exactly where they were (unscoped, zero pre-existing test changes needed)
// -- UpdateEXT syncs them into the default AgeModifier3DEXT's owned ColorInterpolator3DEXT/
// OpacityInterpolator3DEXT and the default LinearGravityModifier3DEXT each call, so the
// public configuration surface for these is unchanged while the new architecture still
// genuinely drives the default per-frame pipeline end-to-end (not bypassed).
//
// AgeEXT/PositionEXT integration stays baked directly in UpdateEXT's own loop, matching 2D's
// own ParticleEmitter::Update, which likewise bakes basic age/position integration in
// directly rather than driving it through a Modifier -- see AgeModifier3DEXT.hpp.
#pragma once

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    class AgeModifier3DEXT;
    class LinearGravityModifier3DEXT;
    class Modifier3DEXT;
    class ModifierExecutionStrategy3DEXT;
    class Profile3DEXT;

    /**
     * @brief Creates, updates, and manages the particles emitted by one emission source.
     * @see ParticleEffect3DEXT, which owns and drives one or more emitters as a single effect.
     * @see ParticleRenderSystem3DEXT, which draws each emitter's active particles as billboards.
     */
    class ParticleEmitter3DEXT
    {
    public:
        ParticleEmitter3DEXT();
        ~ParticleEmitter3DEXT();
        ParticleEmitter3DEXT(ParticleEmitter3DEXT&&) noexcept;
        ParticleEmitter3DEXT& operator=(ParticleEmitter3DEXT&&) noexcept;
        ParticleEmitter3DEXT(const ParticleEmitter3DEXT&) = delete;
        ParticleEmitter3DEXT& operator=(const ParticleEmitter3DEXT&) = delete;

        /** @brief Average number of particles emitted per second while IsEmittingEXT is true. */
        float EmissionRateEXT = 20.0f;

        /** @brief Minimum/maximum lifetime (in seconds) assigned to a newly-emitted particle. */
        float MinLifetimeEXT = 1.0f;
        float MaxLifetimeEXT = 1.0f;

        /** @brief Minimum/maximum initial speed (units/second) assigned to a newly-emitted particle. */
        float MinSpeedEXT = 1.0f;
        float MaxSpeedEXT = 1.0f;

        /** @brief Minimum/maximum initial billboard scale assigned to a newly-emitted particle. */
        float MinScaleEXT = 1.0f;
        float MaxScaleEXT = 1.0f;

        /** @brief Color a particle starts at (age ratio 0). Synced into the default AgeModifier3DEXT's ColorInterpolator3DEXT each UpdateEXT call. */
        Microsoft::Xna::Framework::Color StartColorEXT = Microsoft::Xna::Framework::Color::White;

        /** @brief Color a particle ends at (age ratio 1); linearly interpolated by GetAgeRatioEXT(). */
        Microsoft::Xna::Framework::Color EndColorEXT = Microsoft::Xna::Framework::Color::White;

        /** @brief Opacity a particle starts at (age ratio 0). Synced into the default AgeModifier3DEXT's OpacityInterpolator3DEXT each UpdateEXT call. */
        float StartOpacityEXT = 1.0f;

        /** @brief Opacity a particle ends at (age ratio 1); linearly interpolated by GetAgeRatioEXT(). */
        float EndOpacityEXT = 0.0f;

        /** @brief Constant acceleration (units/second^2) applied to every active particle each frame. Synced into the default LinearGravityModifier3DEXT each UpdateEXT call. */
        Microsoft::Xna::Framework::Vector3 GravityEXT = Microsoft::Xna::Framework::Vector3::Zero;

        /** @brief Maximum number of particles this emitter holds at once; new emissions are dropped once reached. */
        int MaxParticlesEXT = 1000;

        /** @brief Whether this emitter is currently emitting new particles (existing particles still age/update either way). */
        bool IsEmittingEXT = true;

        /** @brief The particles this emitter currently owns. */
        std::vector<Particle3DEXT> ParticlesEXT;

        /** @brief Gets the emission profile computing each new particle's initial offset/heading. Defaults to a ConeProfile3DEXT. */
        [[nodiscard]] Profile3DEXT& getProfileEXTProperty() { return *profileEXT_; }
        /** @brief Replaces the emission profile. Must not be null. */
        void setProfileEXTProperty(std::unique_ptr<Profile3DEXT> profile);

        /** @brief Gets the mutable list of modifiers applied to particles each UpdateEXT call, in order. Defaults to [AgeModifier3DEXT, LinearGravityModifier3DEXT]. */
        [[nodiscard]] std::vector<std::unique_ptr<Modifier3DEXT>>& getModifiersEXTProperty() { return modifiersEXT_; }

        /**
         * @brief Advances every active particle (age/position integration, modifier pipeline,
         * expiry) and emits new particles (accumulated at EmissionRateEXT per second) from
         * @p origin, up to MaxParticlesEXT.
         * @param deltaSeconds Elapsed time since the last UpdateEXT call, in seconds.
         * @param origin World-space position new particles are emitted from.
         */
        void UpdateEXT(float deltaSeconds, const Microsoft::Xna::Framework::Vector3& origin);

        /** @brief Immediately emits @p count new particles from @p origin, ignoring IsEmittingEXT/MaxParticlesEXT accumulation timing (still capped by MaxParticlesEXT). */
        void EmitEXT(int count, const Microsoft::Xna::Framework::Vector3& origin);

    private:
        void SyncBuiltInModifiersEXT();

        std::unique_ptr<Profile3DEXT> profileEXT_;
        std::vector<std::unique_ptr<Modifier3DEXT>> modifiersEXT_;
        ModifierExecutionStrategy3DEXT* modifierExecutionStrategyEXT_ = nullptr;
        AgeModifier3DEXT* defaultAgeModifierEXT_ = nullptr;
        LinearGravityModifier3DEXT* defaultGravityModifierEXT_ = nullptr;
        System::Random randomEXT_;
        float emitAccumulatorEXT_ = 0.0f;
    };
}
