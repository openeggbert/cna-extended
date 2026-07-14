// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleEmitter3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::ParticleEmitter, deliberately scoped down
// (matching this whole plan's repeated "start with the simplest correct version"
// precedent -- see OctreeEXT.hpp/CollisionWorld3DEXT.hpp for the same principle applied
// elsewhere): rather than porting the full Profiles/Modifiers/Interpolators plugin
// architecture (5 emission-shape Profile subclasses, ~13 Modifier subclasses, 6
// Interpolator subclasses in the 2D module), this emitter has ONE built-in emission shape
// (a cone -- ConeHalfAngleEXT == MathHelper::Pi degenerates to full-sphere emission, the
// common "explosion"/"ambient" case) and THREE built-in per-frame behaviors baked directly
// into UpdateEXT: linear gravity, age-based expiry, and start/end color+opacity
// interpolation over each particle's lifetime -- covering AgeModifier's/
// LinearGravityModifier's/ColorInterpolator's/OpacityInterpolator's conceptual roles
// without their extensibility framework. A real, working particle system, just not a
// plugin architecture; the full Profile/Modifier/Interpolator port can be added later if a
// real need appears.
#pragma once

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Random.hpp"

#include <vector>

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Creates, updates, and manages the particles emitted by one emission source.
     * @see ParticleEffect3DEXT, which owns and drives one or more emitters as a single effect.
     * @see ParticleRenderSystem3DEXT, which draws each emitter's active particles as billboards.
     */
    class ParticleEmitter3DEXT
    {
    public:
        ParticleEmitter3DEXT() = default;

        /** @brief Average number of particles emitted per second while IsEmittingEXT is true. */
        float EmissionRateEXT = 20.0f;

        /** @brief Minimum/maximum lifetime (in seconds) assigned to a newly-emitted particle. */
        float MinLifetimeEXT = 1.0f;
        float MaxLifetimeEXT = 1.0f;

        /** @brief Minimum/maximum initial speed (units/second) assigned to a newly-emitted particle. */
        float MinSpeedEXT = 1.0f;
        float MaxSpeedEXT = 1.0f;

        /** @brief Direction the emission cone points, in world space. Normalized internally. */
        Microsoft::Xna::Framework::Vector3 ConeDirectionEXT = Microsoft::Xna::Framework::Vector3::Up;

        /** @brief Half-angle (radians) of the emission cone around ConeDirectionEXT. MathHelper::Pi (the default) emits uniformly over the full sphere. */
        float ConeHalfAngleEXT = 3.14159265358979323846f;

        /** @brief Minimum/maximum initial billboard scale assigned to a newly-emitted particle. */
        float MinScaleEXT = 1.0f;
        float MaxScaleEXT = 1.0f;

        /** @brief Color a particle starts at (age ratio 0). */
        Microsoft::Xna::Framework::Color StartColorEXT = Microsoft::Xna::Framework::Color::White;

        /** @brief Color a particle ends at (age ratio 1); linearly interpolated by GetAgeRatioEXT(). */
        Microsoft::Xna::Framework::Color EndColorEXT = Microsoft::Xna::Framework::Color::White;

        /** @brief Opacity a particle starts at (age ratio 0). */
        float StartOpacityEXT = 1.0f;

        /** @brief Opacity a particle ends at (age ratio 1); linearly interpolated by GetAgeRatioEXT(). */
        float EndOpacityEXT = 0.0f;

        /** @brief Constant acceleration (units/second^2) applied to every active particle each frame. */
        Microsoft::Xna::Framework::Vector3 GravityEXT = Microsoft::Xna::Framework::Vector3::Zero;

        /** @brief Maximum number of particles this emitter holds at once; new emissions are dropped once reached. */
        int MaxParticlesEXT = 1000;

        /** @brief Whether this emitter is currently emitting new particles (existing particles still age/update either way). */
        bool IsEmittingEXT = true;

        /** @brief The particles this emitter currently owns. */
        std::vector<Particle3DEXT> ParticlesEXT;

        /**
         * @brief Advances every active particle (age, gravity/velocity integration, color/opacity
         * interpolation, expiry) and emits new particles (accumulated at EmissionRateEXT per
         * second) from @p origin, up to MaxParticlesEXT.
         * @param deltaSeconds Elapsed time since the last UpdateEXT call, in seconds.
         * @param origin World-space position new particles are emitted from.
         */
        void UpdateEXT(float deltaSeconds, const Microsoft::Xna::Framework::Vector3& origin);

        /** @brief Immediately emits @p count new particles from @p origin, ignoring IsEmittingEXT/MaxParticlesEXT accumulation timing (still capped by MaxParticlesEXT). */
        void EmitEXT(int count, const Microsoft::Xna::Framework::Vector3& origin);

    private:
        [[nodiscard]] Microsoft::Xna::Framework::Vector3 SampleConeDirectionEXT();

        System::Random randomEXT_;
        float emitAccumulatorEXT_ = 0.0f;
    };
}
