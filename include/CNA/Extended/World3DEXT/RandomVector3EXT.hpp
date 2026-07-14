// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::NextUnitVector3EXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::RandomExtensions::NextUnitVector(System::Random&,
// Vector2&) -- a free function extending System::Random, matching that file's own shape,
// rather than a member on System::Random itself (owned by sharp-runtime, not this project)
// or a new FastRandom-style wrapper class (no 3D equivalent of FastRandom exists, and none
// was requested). Factored out here (Phase 11 C-2, 2026-07-14) once enough Profile3DEXT
// subclasses needed the same uniformly-random-heading-over-the-full-sphere sampling
// (PointProfile3DEXT, plus C-2's LineProfile3DEXT/RingProfile3DEXT/BoxProfile3DEXT/
// BoxFillProfile3DEXT/BoxUniformProfile3DEXT) to be worth sharing instead of duplicating the
// z-uniform-in-[-1,1]/phi-uniform-in-[0,2*pi) sampling inline per file -- PointProfile3DEXT's
// own pre-existing inline copy was refactored to call this too (same formula, no behavior
// change).
#pragma once

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
    /** @brief Writes a uniformly-random unit vector (over the full sphere) into @p vector, drawn from @p random. */
    void NextUnitVector3EXT(System::Random& random, Microsoft::Xna::Framework::Vector3& vector);
}
