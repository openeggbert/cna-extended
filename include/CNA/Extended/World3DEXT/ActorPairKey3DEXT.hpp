// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::ActorPairKey3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::ActorPairKey, same shape (an
// order-independent key identifying an unordered actor pair by stable Id, used to
// de-duplicate CollisionWorld3DEXT::QueryCollisionPairs results).
#pragma once

#include <cstddef>
#include <functional>

namespace CNA::Extended::World3DEXT
{
    class ICollisionActor3DEXT;

    /** @brief An order-independent key identifying an unordered pair of collision actors by their stable Id. */
    struct ActorPairKey3DEXT
    {
        int FirstId = 0;
        int SecondId = 0;

        ActorPairKey3DEXT() = default;

        /** @brief Creates a key for the unordered pair (first, second), ordering by Id so pair direction does not affect equality. */
        ActorPairKey3DEXT(const ICollisionActor3DEXT& first, const ICollisionActor3DEXT& second);

        [[nodiscard]] bool Equals(const ActorPairKey3DEXT& other) const { return FirstId == other.FirstId && SecondId == other.SecondId; }

        [[nodiscard]] int GetHashCode() const;

        friend bool operator==(const ActorPairKey3DEXT& left, const ActorPairKey3DEXT& right) { return left.Equals(right); }
        friend bool operator!=(const ActorPairKey3DEXT& left, const ActorPairKey3DEXT& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::World3DEXT::ActorPairKey3DEXT>
    {
        std::size_t operator()(const CNA::Extended::World3DEXT::ActorPairKey3DEXT& value) const noexcept
        {
            return static_cast<std::size_t>(value.GetHashCode());
        }
    };
}
