// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's CollisionShapeKind2D.cs. Upstream's `internal` visibility has
// no C++ equivalent; kept as a regular enum in this namespace, matching the precedent set by
// TransformFlags/PrimitivesHelper elsewhere in this port (documented as an implementation
// detail, not part of the public API contract even though visible). Backing type `byte` ->
// `std::uint8_t`, matching upstream exactly.
#pragma once

#include <cstdint>

namespace CNA::Extended
{
    /** @brief Identifies which concrete shape a CollisionShape2D currently holds. */
    enum class CollisionShapeKind2D : std::uint8_t
    {
        None,
        Box,
        Circle,
        OrientedBox,
        Capsule,
        Polygon
    };
}
