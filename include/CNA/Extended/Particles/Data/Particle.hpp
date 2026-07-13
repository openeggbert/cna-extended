// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/Data/Particle.cs ([StructLayout(LayoutKind.Sequential,
// Pack = 1)] unsafe struct with `fixed float[N]` inline arrays). `fixed` arrays are C#'s workaround
// for embedding raw inline arrays in an unsafe struct -- C++ has this natively as plain array
// members, no `unsafe`/pointer tricks needed. `#pragma pack(push, 1)` reproduces `Pack = 1` (no
// padding between fields), which matters here: `ParticleBuffer` allocates a raw block of
// `SizeInBytes * capacity` and walks it via `Particle*` pointer arithmetic, so the actual in-memory
// size and field offsets must match what upstream's tight packing produces.
#pragma once

#include <cstddef>

namespace CNA::Extended::Particles::Data
{
#pragma pack(push, 1)
    /** @brief An individual particle: tightly-packed fixed-layout data walked via raw pointer arithmetic in ParticleBuffer. */
    struct Particle
    {
        /** @brief The time (in seconds) when this particle was created. */
        float Inception;

        /** @brief The current age (in seconds) of this particle. */
        float Age;

        /** @brief The current position of this particle in 2D space [X, Y]. */
        float Position[2];

        /** @brief The current velocity vector of this particle [X, Y]. */
        float Velocity[2];

        /** @brief The color of this particle in RGB format [R, G, B]. */
        float Color[3];

        /** @brief The scale factor applied to this particle's visual representation. */
        float Scale[2];

        /** @brief The position where this particle was triggered or emitted from [X, Y]. */
        float TriggeredPos[2];

        /** @brief The opacity (alpha) value of this particle, ranging from 0.0 (transparent) to 1.0 (opaque). */
        float Opacity;

        /** @brief The rotation of this particle in radians. */
        float Rotation;

        /** @brief The mass of this particle used during physics calculations. */
        float Mass;

        /** @brief The depth at which this particle is rendered (0.0 front to 1.0 back), used for layering. */
        float LayerDepth;

        /** @brief The size of the Particle struct in bytes; used for memory allocations and buffer operations. */
        static const std::size_t SizeInBytes;
    };
#pragma pack(pop)

    // Defined outside the class body: `sizeof(Particle)` is only valid once Particle is a complete
    // type, which it isn't yet inside its own definition above.
    inline const std::size_t Particle::SizeInBytes = sizeof(Particle);
}
