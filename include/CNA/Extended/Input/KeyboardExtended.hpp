// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/KeyboardExtended.cs: a process-wide keyboard state
// tracker offering a KeyboardStateExtended snapshot combining the current and previous frame's
// KeyboardState. Upstream's `static` fields -> C++ `static` class members, matching this
// project's established convention for C# static utility classes (e.g. Collision2D).
#pragma once

#include "CNA/Extended/Input/KeyboardStateExtended.hpp"

namespace CNA::Extended::Input
{
    /** @brief Tracks keyboard input across frames, offering both the previous and current KeyboardState via GetState(). */
    class KeyboardExtended
    {
    public:
        KeyboardExtended() = delete;

        /** @brief Gets the current state of keyboard input, combining this and the previous frame's KeyboardState. */
        [[nodiscard]] static KeyboardStateExtended GetState();

        /**
         * @brief Updates the tracked keyboard state. Call exactly once per update cycle -- calling
         * it more than once per cycle overwrites the cached previous state with invalid data.
         */
        static void Update();

    private:
        static KeyboardState currentKeyboardState_;
        static KeyboardState previousKeyboardState_;
    };
}
