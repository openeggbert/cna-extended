// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/LayerPair.cs: an order-independent key identifying an
// unordered pair of Layer instances (used to record which layer pairs have collision enabled).
//
// Upstream orders First/Second using `first.GetHashCode() <= second.GetHashCode()` -- NOT a
// meaningful value comparison, just upstream's own way of getting an arbitrary-but-consistent
// order for any two Layer references (Layer never overrides GetHashCode(), so this is C#'s default
// per-object identity hash, stable for an object's lifetime but not tied to any field). Equals()
// then compares by ReferenceEquals, not by the ordered fields' values -- so the ordering step exists
// purely to make (a, b) and (b, a) hash/compare identically, not to express any real ordering
// relationship between layers.
//
// Ported as: order First/Second by raw pointer address instead of a fabricated identity hash --
// pointer addresses are C++'s own stable, always-available per-object identity, serving exactly
// the same "arbitrary but consistent" role upstream's identity-hash comparison does, without
// needing to invent a fake GetHashCode() for Layer (which has no upstream override to preserve).
// Equals()/GetHashCode() below likewise compare/hash the stored Layer* pointers directly (pointer
// equality = C#'s ReferenceEquals; std::hash<const Layer*> = a stable per-object hash, matching
// the *intent* of C#'s identity hash even though the concrete values differ).
//
// Upstream's `internal readonly struct` -- kept public, matching the ActorPairKey/
// CollisionShapeKind2D internal-has-no-C++-equivalent precedent. Constructor takes Layer by const
// reference (not by pointer) since C++ references cannot be null, dropping upstream's
// ArgumentNullException.ThrowIfNull checks -- same convention as ActorPairKey.hpp.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace CNA::Extended::Collisions
{
    class Layer;

    /** @brief An order-independent key identifying an unordered pair of collision layers by pointer identity. */
    struct LayerPair
    {
        const Layer* First = nullptr;
        const Layer* Second = nullptr;

        LayerPair() = default;

        /** @brief Creates a key for the unordered pair (first, second), ordering by address so pair direction does not affect equality. */
        LayerPair(const Layer& first, const Layer& second);

        [[nodiscard]] bool Equals(const LayerPair& other) const { return First == other.First && Second == other.Second; }

        [[nodiscard]] std::size_t GetHashCode() const;

        friend bool operator==(const LayerPair& left, const LayerPair& right) { return left.Equals(right); }
        friend bool operator!=(const LayerPair& left, const LayerPair& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::Collisions::LayerPair>
    {
        std::size_t operator()(const CNA::Extended::Collisions::LayerPair& value) const noexcept { return value.GetHashCode(); }
    };
}
