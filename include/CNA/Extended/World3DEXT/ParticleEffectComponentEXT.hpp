// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ParticleEffectComponentEXT -- new, non-upstream addition.
// See 3d.md/plan3d.md at the repository root for the design.
//
// ECS component pairing a non-owning ParticleEffect3DEXT* with the texture its particles
// are drawn with. One texture per effect (not per-particle/per-emitter) -- the simplest
// correct scope for this phase; a caller wanting different textures per emitter can attach
// multiple entities, each with its own ParticleEffectComponentEXT.
#pragma once

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::World3DEXT
{
    class ParticleEffect3DEXT;

    /**
     * @brief ECS component pairing a non-owning ParticleEffect3DEXT* with the texture its
     * particles are drawn with.
     * @see ParticleUpdateSystem3DEXT, which advances EffectEXT each frame.
     * @see ParticleRenderSystem3DEXT, which draws EffectEXT's active particles as billboards.
     */
    struct ParticleEffectComponentEXT
    {
        /** @brief The effect to update/draw. Not owned by this component. */
        ParticleEffect3DEXT* EffectEXT = nullptr;

        /** @brief The texture every particle in EffectEXT is drawn with. Not owned by this component. */
        Microsoft::Xna::Framework::Graphics::Texture2D* TextureEXT = nullptr;
    };
}
