// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/KeyboardModifiers.cs. Upstream lives in
// the MonoGame.Extended.Input.InputListeners namespace, distinct from MonoGame.Extended.Input
// (where KeyboardExtended/MouseExtended/etc. live) -- ported into a matching
// CNA::Extended::Input::InputListeners sub-namespace, mirroring upstream's own namespace split
// (the same convention already applied to CNA::Extended::Collisions/CNA::Extended::Timers).
// `[Flags]` bitwise operators -> free-function operators, matching MouseButton.hpp's pattern.
#pragma once

#include <type_traits>

namespace CNA::Extended::Input::InputListeners
{
    /** @brief Identifies one or more active keyboard modifier keys (Control/Shift/Alt). */
    enum class KeyboardModifiers
    {
        Control = 1,
        Shift = 2,
        Alt = 4,
        None = 0
    };

    [[nodiscard]] constexpr KeyboardModifiers operator|(KeyboardModifiers left, KeyboardModifiers right)
    {
        using U = std::underlying_type_t<KeyboardModifiers>;
        return static_cast<KeyboardModifiers>(static_cast<U>(left) | static_cast<U>(right));
    }

    [[nodiscard]] constexpr KeyboardModifiers operator&(KeyboardModifiers left, KeyboardModifiers right)
    {
        using U = std::underlying_type_t<KeyboardModifiers>;
        return static_cast<KeyboardModifiers>(static_cast<U>(left) & static_cast<U>(right));
    }

    constexpr KeyboardModifiers& operator|=(KeyboardModifiers& left, KeyboardModifiers right)
    {
        left = left | right;
        return left;
    }
}
