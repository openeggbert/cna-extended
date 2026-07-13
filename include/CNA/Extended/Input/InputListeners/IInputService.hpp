// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/IInputService.cs: a small service-locator
// interface exposing the shared GUI input listeners. Pure abstract interface, matching this
// project's established pure-interface pattern (see ICollisionActor.hpp). Upstream's
// reference-type properties (`KeyboardListener GuiKeyboardListener { get; }`, etc.) -> non-owning
// raw pointer getters, matching this whole module's established "caller/service owns, this is
// just a lookup" convention.
#pragma once

namespace CNA::Extended::Input::InputListeners
{
    class KeyboardListener;
    class MouseListener;
    class GamePadListener;
    class TouchListener;

    /** @brief Provides access to the shared GUI input listeners for a game. */
    class IInputService
    {
    public:
        virtual ~IInputService() = default;

        /** @brief Gets the shared keyboard listener used for GUI input. */
        [[nodiscard]] virtual KeyboardListener* getGuiKeyboardListenerProperty() const = 0;

        /** @brief Gets the shared mouse listener used for GUI input. */
        [[nodiscard]] virtual MouseListener* getGuiMouseListenerProperty() const = 0;

        /** @brief Gets the shared gamepad listener used for GUI input. */
        [[nodiscard]] virtual GamePadListener* getGuiGamePadListenerProperty() const = 0;

        /** @brief Gets the shared touch listener used for GUI input. */
        [[nodiscard]] virtual TouchListener* getGuiTouchListenerProperty() const = 0;
    };
}
