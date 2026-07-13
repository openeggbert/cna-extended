// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/SpriteBatchExtensions.ParticleEfffect.cs (upstream's
// own filename typo, "Efffect" with 3 f's -- the actual class/namespace names are correctly
// spelled). Extension methods -> free functions in this namespace, matching this project's
// established convention (see Graphics/SpriteBatchExtensions.hpp, a *different* type in a
// different namespace -- no collision). `SpriteBatch`/`ParticleEffect`/`ParticleEmitter` params
// are references, not pointers (non-null by construction in C++), so upstream's
// `ArgumentNullException.ThrowIfNull` checks are dropped, matching this project's established
// reference-type-parameter convention. `Span<IntPtr>`/`stackalloc` (the FrontToBack path's
// reverse-iteration buffer) -> `std::vector<Data::Particle*>`; this project has no established
// stack-allocation idiom and the upstream stackalloc is purely a micro-optimization for the
// <=1024-particle case, not behavior-affecting.
#pragma once

namespace Microsoft::Xna::Framework::Graphics
{
    class SpriteBatch;
}

namespace CNA::Extended::Particles
{
    class ParticleEffect;
    class ParticleEmitter;

    /**
     * @brief Draws every active emitter in @p effect.
     * @throws System::ObjectDisposedException effect was already disposed.
     */
    void Draw(Microsoft::Xna::Framework::Graphics::SpriteBatch& spriteBatch, ParticleEffect& effect);

    /**
     * @brief Draws every active particle in @p emitter.
     * @throws System::ObjectDisposedException emitter was already disposed.
     */
    void Draw(Microsoft::Xna::Framework::Graphics::SpriteBatch& spriteBatch, ParticleEmitter& emitter);
}
