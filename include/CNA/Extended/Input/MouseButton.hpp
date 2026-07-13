// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/MouseButton.cs. Upstream's `[Flags]` bitwise operators
// -> free-function operators on the underlying type, matching CNA's own established pattern for
// flags enums (see Microsoft::Xna::Framework::Input::Buttons's operator|/operator&/operator~).
#pragma once

#include <type_traits>

namespace CNA::Extended::Input
{
    /** @brief Identifies one or more mouse buttons. */
    enum class MouseButton
    {
        None = 0,
        Left = 1,
        Middle = 2,
        Right = 4,
        XButton1 = 8,
        XButton2 = 16
    };

    [[nodiscard]] constexpr MouseButton operator|(MouseButton left, MouseButton right)
    {
        using U = std::underlying_type_t<MouseButton>;
        return static_cast<MouseButton>(static_cast<U>(left) | static_cast<U>(right));
    }

    [[nodiscard]] constexpr MouseButton operator&(MouseButton left, MouseButton right)
    {
        using U = std::underlying_type_t<MouseButton>;
        return static_cast<MouseButton>(static_cast<U>(left) & static_cast<U>(right));
    }

    constexpr MouseButton& operator|=(MouseButton& left, MouseButton right)
    {
        left = left | right;
        return left;
    }
}
