// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ParticleEffect3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    ParticleEmitter3DEXT& ParticleEffect3DEXT::AddEmitterEXT(std::unique_ptr<ParticleEmitter3DEXT> emitter)
    {
        emittersEXT_.push_back(std::move(emitter));
        return *emittersEXT_.back();
    }

    void ParticleEffect3DEXT::UpdateEXT(float deltaSeconds, const Vector3& origin)
    {
        for (const std::unique_ptr<ParticleEmitter3DEXT>& emitter : emittersEXT_)
        {
            emitter->UpdateEXT(deltaSeconds, origin);
        }
    }
}
