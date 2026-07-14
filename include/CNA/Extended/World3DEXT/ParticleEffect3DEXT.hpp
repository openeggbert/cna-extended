// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleEffect3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md and ParticleEmitter3DEXT.hpp's own header comment for the design.
//
// 3D counterpart of CNA::Extended::Particles::ParticleEffect: owns one or more
// ParticleEmitter3DEXT instances and drives them together as a single logical effect
// (e.g. a torch = one "flame" emitter + one "smoke" emitter, updated/emitted from the
// same world position).
#pragma once

#include "CNA/Extended/World3DEXT/ParticleEmitter3DEXT.hpp"

#include <memory>
#include <vector>

namespace CNA::Extended::World3DEXT
{
    /**
     * @brief Owns and drives one or more ParticleEmitter3DEXT instances as a single effect.
     * @see ParticleEffectComponentEXT, the ECS component pairing this with a world position/texture.
     */
    class ParticleEffect3DEXT
    {
    public:
        ParticleEffect3DEXT() = default;

        /** @brief Adds and takes ownership of a new emitter, returning a reference to it for further configuration. */
        ParticleEmitter3DEXT& AddEmitterEXT(std::unique_ptr<ParticleEmitter3DEXT> emitter);

        /** @brief Gets every emitter this effect owns. */
        [[nodiscard]] const std::vector<std::unique_ptr<ParticleEmitter3DEXT>>& getEmittersProperty() const { return emittersEXT_; }

        /** @brief Advances every owned emitter (see ParticleEmitter3DEXT::UpdateEXT). */
        void UpdateEXT(float deltaSeconds, const Microsoft::Xna::Framework::Vector3& origin);

    private:
        std::vector<std::unique_ptr<ParticleEmitter3DEXT>> emittersEXT_;
    };
}
