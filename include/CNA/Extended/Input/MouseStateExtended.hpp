// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/MouseStateExtended.cs. `Func<MouseState, ButtonState>`
// private helper parameters -> `std::function<ButtonState(const MouseState&)>`, matching this
// project's established Func/Action-to-std::function convention.
#pragma once

#include "CNA/Extended/Input/MouseButton.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"

#include <functional>

namespace CNA::Extended::Input
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::ButtonState;
    using Microsoft::Xna::Framework::Input::MouseState;

    /**
     * @brief An extended view of mouse input state, tracking both the previous and current
     * MouseState to expose position-delta and "was this button just pressed/released" queries.
     */
    struct MouseStateExtended
    {
        MouseStateExtended() = default;

        /**
         * @brief Initializes a new MouseStateExtended value.
         * @param currentMouseState The state of mouse input during the current update cycle.
         * @param previousMouseState The state of mouse input during the previous update cycle.
         */
        MouseStateExtended(const MouseState& currentMouseState, const MouseState& previousMouseState);

        /** @brief Gets the current x-coordinate position of the mouse cursor relative to the game window. */
        [[nodiscard]] int getXProperty() const;

        /** @brief Gets the current y-coordinate position of the mouse cursor relative to the game window. */
        [[nodiscard]] int getYProperty() const;

        /** @brief Gets the current xy-coordinate position of the mouse cursor relative to the game window. */
        [[nodiscard]] Point getPositionProperty() const;

        /** @brief Gets whether the position of the mouse cursor changed between the previous and current states. */
        [[nodiscard]] bool getPositionChangedProperty() const;

        /** @brief Gets the difference in the x-coordinate position of the mouse between the previous and current state. */
        [[nodiscard]] int getDeltaXProperty() const;

        /** @brief Gets the difference in the y-coordinate position of the mouse between the previous and current state. */
        [[nodiscard]] int getDeltaYProperty() const;

        /** @brief Gets the difference in the xy-coordinate position of the mouse between the previous and current state. */
        [[nodiscard]] Point getDeltaPositionProperty() const;

        /** @brief Gets the current value of the mouse scroll wheel. */
        [[nodiscard]] int getScrollWheelValueProperty() const;

        /** @brief Gets the difference in the mouse scroll wheel value between the previous and current state. */
        [[nodiscard]] int getDeltaScrollWheelValueProperty() const;

        /** @brief Gets the current state of the mouse left button. */
        [[nodiscard]] ButtonState getLeftButtonProperty() const;

        /** @brief Gets the current state of the mouse middle button. */
        [[nodiscard]] ButtonState getMiddleButtonProperty() const;

        /** @brief Gets the current state of the mouse right button. */
        [[nodiscard]] ButtonState getRightButtonProperty() const;

        /** @brief Gets the current state of the first mouse extra button. */
        [[nodiscard]] ButtonState getXButton1Property() const;

        /** @brief Gets the current state of the second mouse extra button. */
        [[nodiscard]] ButtonState getXButton2Property() const;

        /** @brief Returns whether the specified mouse button is down during the current state. */
        [[nodiscard]] bool IsButtonDown(MouseButton button) const;

        /** @brief Returns whether the specified mouse button is up during the current state. */
        [[nodiscard]] bool IsButtonUp(MouseButton button) const;

        /** @brief Returns whether the specified mouse button was up during the previous state, but is now down. */
        [[nodiscard]] bool WasButtonPressed(MouseButton button) const;

        /** @brief Returns whether the specified mouse button was down during the previous state, but is now up. */
        [[nodiscard]] bool WasButtonReleased(MouseButton button) const;

    private:
        [[nodiscard]] bool IsPressed(const std::function<ButtonState(const MouseState&)>& button) const;
        [[nodiscard]] bool IsReleased(const std::function<ButtonState(const MouseState&)>& button) const;
        [[nodiscard]] bool WasJustPressed(const std::function<ButtonState(const MouseState&)>& button) const;
        [[nodiscard]] bool WasJustReleased(const std::function<ButtonState(const MouseState&)>& button) const;

        MouseState currentMouseState_;
        MouseState previousMouseState_;
    };
}
