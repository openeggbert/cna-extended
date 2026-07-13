// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Collisions/ActorPairKey.cs: an order-independent key
// identifying an unordered actor pair (used to de-duplicate collision-pair query results).
// Upstream's `internal` visibility has no C++ equivalent; kept public, matching the precedent set
// by CollisionResult2D::Invert()/CollisionShapeKind2D elsewhere in this port. The constructor
// takes ICollisionActor by const reference rather than by pointer -- C++ references cannot be
// null, so upstream's `ArgumentNullException.ThrowIfNull(first)`/`ThrowIfNull(second)` checks have
// no C++ equivalent to preserve and are naturally dropped, matching how this project translates
// non-nullable C# reference-type parameters elsewhere.
#pragma once

#include <cstddef>
#include <functional>

namespace CNA::Extended::Collisions
{
    class ICollisionActor;

    /** @brief An order-independent key identifying an unordered pair of collision actors by their stable Id. */
    struct ActorPairKey
    {
        int FirstId = 0;
        int SecondId = 0;

        ActorPairKey() = default;

        /** @brief Creates a key for the unordered pair (first, second), ordering by Id so pair direction does not affect equality. */
        ActorPairKey(const ICollisionActor& first, const ICollisionActor& second);

        [[nodiscard]] bool Equals(const ActorPairKey& other) const { return FirstId == other.FirstId && SecondId == other.SecondId; }

        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const ActorPairKey& left, const ActorPairKey& right) { return left.Equals(right); }
        friend bool operator!=(const ActorPairKey& left, const ActorPairKey& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::Collisions::ActorPairKey>
    {
        std::size_t operator()(const CNA::Extended::Collisions::ActorPairKey& value) const noexcept
        {
            return static_cast<std::size_t>(value.GetHashCode());
        }
    };
}
