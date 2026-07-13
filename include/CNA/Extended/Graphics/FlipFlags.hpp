// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/FlipFlags.cs. `[Flags]` bitwise operators -> free
// function operators, matching this project's established pattern (see
// Input/InputListeners/KeyboardModifiers.hpp).
#pragma once

#include <cstdint>
#include <type_traits>

namespace CNA::Extended::Graphics
{
    /** @brief Identifies which axes a sprite/texture region is flipped on. */
    enum class FlipFlags : std::uint8_t
    {
        None = 0,
        FlipDiagonally = 1 << 0,
        FlipVertically = 1 << 1,
        FlipHorizontally = 1 << 2
    };

    [[nodiscard]] constexpr FlipFlags operator|(FlipFlags left, FlipFlags right)
    {
        using U = std::underlying_type_t<FlipFlags>;
        return static_cast<FlipFlags>(static_cast<U>(left) | static_cast<U>(right));
    }

    [[nodiscard]] constexpr FlipFlags operator&(FlipFlags left, FlipFlags right)
    {
        using U = std::underlying_type_t<FlipFlags>;
        return static_cast<FlipFlags>(static_cast<U>(left) & static_cast<U>(right));
    }

    constexpr FlipFlags& operator|=(FlipFlags& left, FlipFlags right)
    {
        left = left | right;
        return left;
    }
}
