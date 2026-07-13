// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ECS/BitArrayExtensions.cs. Extension methods -> free functions
// in this namespace, matching this project's established convention. Note: within this module,
// runtime component tracking actually uses `ComponentBits` (a fixed 256-bit field), not
// `System::Collections::BitArray` -- these functions operate on general-purpose `BitArray`
// instances a caller might use elsewhere, matching upstream's own scope (this file is not
// referenced by any other file in ECS/ upstream either).
#pragma once

#include "System/Collections/BitArray.hpp"

namespace CNA::Extended::ECS
{
    using System::Collections::BitArray;

    /** @brief Determines whether every bit in @p bitArray is unset. */
    [[nodiscard]] bool IsEmpty(const BitArray& bitArray);

    /** @brief Determines whether @p bitArray has every bit set that @p other has set. */
    [[nodiscard]] bool ContainsAll(const BitArray& bitArray, const BitArray& other);

    /** @brief Determines whether @p bitArray and @p other have at least one set bit in common. */
    [[nodiscard]] bool Intersects(const BitArray& bitArray, const BitArray& other);
}
