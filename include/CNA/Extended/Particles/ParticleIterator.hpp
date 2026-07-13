// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleIterator.cs. Holds a non-owning back-pointer
// to the ParticleBuffer it iterates (matching upstream's aliased reference) -- ParticleBuffer owns
// exactly one ParticleIterator instance for its lifetime and hands out `Iterator` (a Reset() call
// on that same instance) rather than allocating a fresh iterator per call, exactly as upstream
// does. `ParticleBuffer` is only forward-declared here (a pointer member doesn't need the complete
// type); the .cpp includes the real header. Upstream's `internal Reset(int offset)` (assembly-only
// visibility) is kept public, matching this project's established internal-visibility convention.
#pragma once

#include "CNA/Extended/Particles/Data/Particle.hpp"

namespace CNA::Extended::Particles
{
    class ParticleBuffer;

    /** @brief Iterates the active particles in a ParticleBuffer's circular buffer, handling wraparound. */
    class ParticleIterator
    {
    public:
        /** @throws std::invalid_argument buffer is null. @throws System::ObjectDisposedException buffer was already disposed. */
        explicit ParticleIterator(ParticleBuffer* buffer);

        /** @brief Gets the total number of particles iterable from the position set by the last Reset(). */
        [[nodiscard]] int getTotalProperty() const { return total_; }

        /** @brief Gets whether there are more particles to iterate. */
        [[nodiscard]] bool getHasNextProperty() const;

        /** @brief Resets the iterator to the buffer's current head (start of active particles). */
        ParticleIterator& Reset();

        /** @brief Resets the iterator to @p offset particles past the buffer's head, handling wraparound. */
        ParticleIterator& Reset(int offset);

        /** @brief Returns a pointer to the current particle and advances the iterator, wrapping at the buffer's end. */
        [[nodiscard]] Data::Particle* Next();

    private:
        ParticleBuffer* buffer_;
        Data::Particle* current_ = nullptr;
        int total_ = 0;
    };
}
