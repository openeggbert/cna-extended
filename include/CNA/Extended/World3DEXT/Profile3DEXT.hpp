// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Profile3DEXT -- new, non-upstream addition. See 3d.md/plan3d.md
// at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Particles::Profiles::Profile, brought to parity
// (2026-07-14, user-requested): the first piece of the full Profiles/Modifiers/Interpolators
// plugin architecture ParticleEmitter3DEXT previously lacked (see ParticleEmitter3DEXT.hpp's
// own header comment, now updated, for the "why scoped down originally" history).
//
// One deliberate signature deviation from the 2D 1:1 mirror: GetOffsetAndHeading takes a
// System::Random& parameter that upstream's Profile doesn't have (2D's concrete Profiles
// draw from a global CNA::Extended::FastRandom::getSharedProperty() singleton instead).
// World3DEXT has no equivalent global RNG and, more importantly, this project's established
// 3D particle convention (ParticleEmitter3DEXT::randomEXT_) already keeps randomness
// instance-owned per emitter for reproducibility -- a global shared RNG here would silently
// decorrelate a Profile's draws from the rest of that same emitter's own random sequence
// (lifetime/speed/scale sampling), which isn't behaviorally necessary and is worth avoiding.
#pragma once

#include <memory>

namespace Microsoft::Xna::Framework
{
    struct Vector3;
}

namespace System
{
    class Random;
}

namespace CNA::Extended::World3DEXT
{
    /** @brief Base class for particle emission profiles: computes the initial offset and heading for each newly released particle. */
    class Profile3DEXT
    {
    public:
        virtual ~Profile3DEXT() = default;

        /** @brief Computes the offset (from the emitter position) and unit heading for a new particle, writing into @p offset/@p heading. */
        virtual void GetOffsetAndHeading(Microsoft::Xna::Framework::Vector3* offset, Microsoft::Xna::Framework::Vector3* heading,
                                          System::Random& random) = 0;

        /** @brief Creates a profile emitting all particles from a single point with a uniformly random heading over the full sphere. */
        [[nodiscard]] static std::unique_ptr<Profile3DEXT> Point();

        /**
         * @brief Creates a profile emitting all particles from a single point in a directional cone.
         * @note NOXNA -- not a 2D Profile port (no 2D Profile maps cleanly onto a 3D solid cone
         * distribution). Reuses the exact sampling algorithm ParticleEmitter3DEXT used directly
         * before this architecture existed (see ConeProfile3DEXT.hpp).
         */
        [[nodiscard]] static std::unique_ptr<Profile3DEXT> Cone(const Microsoft::Xna::Framework::Vector3& direction, float halfAngle);
    };
}
