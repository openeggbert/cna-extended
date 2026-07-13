// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/MouseExtended.cs. Upstream's `#if !FNA` SetCursor overload
// is omitted upstream when mirroring FNA, but CNA's own Mouse::SetCursor is tagged NOXNA (CNA
// provides it regardless of the FNA-mirroring convention) -- so SetCursor is ported here too,
// matching what CNA actually offers rather than upstream's own FNA/non-FNA split. `IntPtr
// WindowHandle` -> `std::uintptr_t`, matching CNA's own Mouse::getWindowHandleProperty()/
// setWindowHandleProperty() representation.
#pragma once

#include "CNA/Extended/Input/MouseStateExtended.hpp"
#include "Microsoft/Xna/Framework/Input/MouseCursor.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"

#include <cstdint>

namespace CNA::Extended::Input
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::MouseCursor;

    /** @brief Tracks mouse input across frames, offering both the previous and current MouseState via GetState(). */
    class MouseExtended
    {
    public:
        MouseExtended() = delete;

        /** @brief Gets the current state of mouse input, combining this and the previous frame's MouseState. */
        [[nodiscard]] static MouseStateExtended GetState();

        /**
         * @brief Updates the tracked mouse state. Call exactly once per update cycle -- calling it
         * more than once per cycle overwrites the cached previous state with invalid data.
         */
        static void Update();

        /** @brief Sets the position of the mouse cursor to the specified coordinates relative to the game window. */
        static void SetPosition(int x, int y);

        /** @brief Sets the position of the mouse cursor to the specified coordinate relative to the game window. */
        static void SetPosition(const Point& point);

        /** @brief Sets the cursor of the mouse. */
        static void SetCursor(MouseCursor& cursor);

        /** @brief Gets the window handle of the mouse. */
        [[nodiscard]] static std::uintptr_t getWindowHandleProperty();

        /** @brief Sets the window handle of the mouse. */
        static void setWindowHandleProperty(std::uintptr_t value);

    private:
        static MouseState currentMouseState_;
        static MouseState previousMouseState_;
    };
}
