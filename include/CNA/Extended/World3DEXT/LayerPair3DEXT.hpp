// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::LayerPair3DEXT -- new, non-upstream addition. See
// 3d.md/plan3d.md at the repository root for the design.
//
// 3D counterpart of CNA::Extended::Collisions::LayerPair, same shape with Layer3DEXT in
// place of Layer -- an order-independent key identifying an unordered pair of collision
// layers by pointer identity (see LayerPair.hpp's own header comment for why pointer
// address, not a fabricated identity hash, is the right ordering key in C++).
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace CNA::Extended::World3DEXT
{
    class Layer3DEXT;

    /** @brief An order-independent key identifying an unordered pair of collision layers by pointer identity. */
    struct LayerPair3DEXT
    {
        const Layer3DEXT* First = nullptr;
        const Layer3DEXT* Second = nullptr;

        LayerPair3DEXT() = default;

        /** @brief Creates a key for the unordered pair (first, second), ordering by address so pair direction does not affect equality. */
        LayerPair3DEXT(const Layer3DEXT& first, const Layer3DEXT& second);

        [[nodiscard]] bool Equals(const LayerPair3DEXT& other) const { return First == other.First && Second == other.Second; }

        [[nodiscard]] std::size_t GetHashCode() const;

        friend bool operator==(const LayerPair3DEXT& left, const LayerPair3DEXT& right) { return left.Equals(right); }
        friend bool operator!=(const LayerPair3DEXT& left, const LayerPair3DEXT& right) { return !left.Equals(right); }
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::World3DEXT::LayerPair3DEXT>
    {
        std::size_t operator()(const CNA::Extended::World3DEXT::LayerPair3DEXT& value) const noexcept { return value.GetHashCode(); }
    };
}
