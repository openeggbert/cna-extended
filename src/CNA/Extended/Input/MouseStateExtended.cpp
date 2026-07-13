// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/MouseStateExtended.hpp"

namespace CNA::Extended::Input
{
    MouseStateExtended::MouseStateExtended(const MouseState& currentMouseState, const MouseState& previousMouseState)
        : currentMouseState_(currentMouseState), previousMouseState_(previousMouseState)
    {
    }

    int MouseStateExtended::getXProperty() const
    {
        return currentMouseState_.getXProperty();
    }

    int MouseStateExtended::getYProperty() const
    {
        return currentMouseState_.getYProperty();
    }

    Point MouseStateExtended::getPositionProperty() const
    {
        return Point(currentMouseState_.getXProperty(), currentMouseState_.getYProperty());
    }

    bool MouseStateExtended::getPositionChangedProperty() const
    {
        return currentMouseState_.getXProperty() != previousMouseState_.getXProperty()
            || currentMouseState_.getYProperty() != previousMouseState_.getYProperty();
    }

    int MouseStateExtended::getDeltaXProperty() const
    {
        return previousMouseState_.getXProperty() - currentMouseState_.getXProperty();
    }

    int MouseStateExtended::getDeltaYProperty() const
    {
        return previousMouseState_.getYProperty() - currentMouseState_.getYProperty();
    }

    Point MouseStateExtended::getDeltaPositionProperty() const
    {
        return Point(getDeltaXProperty(), getDeltaYProperty());
    }

    int MouseStateExtended::getScrollWheelValueProperty() const
    {
        return currentMouseState_.getScrollWheelValueProperty();
    }

    int MouseStateExtended::getDeltaScrollWheelValueProperty() const
    {
        return previousMouseState_.getScrollWheelValueProperty() - currentMouseState_.getScrollWheelValueProperty();
    }

    ButtonState MouseStateExtended::getLeftButtonProperty() const
    {
        return currentMouseState_.getLeftButtonProperty();
    }

    ButtonState MouseStateExtended::getMiddleButtonProperty() const
    {
        return currentMouseState_.getMiddleButtonProperty();
    }

    ButtonState MouseStateExtended::getRightButtonProperty() const
    {
        return currentMouseState_.getRightButtonProperty();
    }

    ButtonState MouseStateExtended::getXButton1Property() const
    {
        return currentMouseState_.getXButton1Property();
    }

    ButtonState MouseStateExtended::getXButton2Property() const
    {
        return currentMouseState_.getXButton2Property();
    }

    bool MouseStateExtended::IsButtonDown(MouseButton button) const
    {
        switch (button)
        {
            case MouseButton::Left:
                return IsPressed([](const MouseState& m) { return m.getLeftButtonProperty(); });
            case MouseButton::Middle:
                return IsPressed([](const MouseState& m) { return m.getMiddleButtonProperty(); });
            case MouseButton::Right:
                return IsPressed([](const MouseState& m) { return m.getRightButtonProperty(); });
            case MouseButton::XButton1:
                return IsPressed([](const MouseState& m) { return m.getXButton1Property(); });
            case MouseButton::XButton2:
                return IsPressed([](const MouseState& m) { return m.getXButton2Property(); });
            default:
                break;
        }

        return false;
    }

    bool MouseStateExtended::IsButtonUp(MouseButton button) const
    {
        switch (button)
        {
            case MouseButton::Left:
                return IsReleased([](const MouseState& m) { return m.getLeftButtonProperty(); });
            case MouseButton::Middle:
                return IsReleased([](const MouseState& m) { return m.getMiddleButtonProperty(); });
            case MouseButton::Right:
                return IsReleased([](const MouseState& m) { return m.getRightButtonProperty(); });
            case MouseButton::XButton1:
                return IsReleased([](const MouseState& m) { return m.getXButton1Property(); });
            case MouseButton::XButton2:
                return IsReleased([](const MouseState& m) { return m.getXButton2Property(); });
            default:
                break;
        }

        return false;
    }

    bool MouseStateExtended::WasButtonPressed(MouseButton button) const
    {
        switch (button)
        {
            case MouseButton::Left:
                return WasJustPressed([](const MouseState& m) { return m.getLeftButtonProperty(); });
            case MouseButton::Middle:
                return WasJustPressed([](const MouseState& m) { return m.getMiddleButtonProperty(); });
            case MouseButton::Right:
                return WasJustPressed([](const MouseState& m) { return m.getRightButtonProperty(); });
            case MouseButton::XButton1:
                return WasJustPressed([](const MouseState& m) { return m.getXButton1Property(); });
            case MouseButton::XButton2:
                return WasJustPressed([](const MouseState& m) { return m.getXButton2Property(); });
            default:
                return false;
        }
    }

    bool MouseStateExtended::WasButtonReleased(MouseButton button) const
    {
        switch (button)
        {
            case MouseButton::Left:
                return WasJustReleased([](const MouseState& m) { return m.getLeftButtonProperty(); });
            case MouseButton::Middle:
                return WasJustReleased([](const MouseState& m) { return m.getMiddleButtonProperty(); });
            case MouseButton::Right:
                return WasJustReleased([](const MouseState& m) { return m.getRightButtonProperty(); });
            case MouseButton::XButton1:
                return WasJustReleased([](const MouseState& m) { return m.getXButton1Property(); });
            case MouseButton::XButton2:
                return WasJustReleased([](const MouseState& m) { return m.getXButton2Property(); });
            default:
                return false;
        }
    }

    bool MouseStateExtended::IsPressed(const std::function<ButtonState(const MouseState&)>& button) const
    {
        return button(currentMouseState_) == ButtonState::Pressed;
    }

    bool MouseStateExtended::IsReleased(const std::function<ButtonState(const MouseState&)>& button) const
    {
        return button(currentMouseState_) == ButtonState::Released;
    }

    bool MouseStateExtended::WasJustPressed(const std::function<ButtonState(const MouseState&)>& button) const
    {
        return button(previousMouseState_) == ButtonState::Released && button(currentMouseState_) == ButtonState::Pressed;
    }

    bool MouseStateExtended::WasJustReleased(const std::function<ButtonState(const MouseState&)>& button) const
    {
        return button(previousMouseState_) == ButtonState::Pressed && button(currentMouseState_) == ButtonState::Released;
    }
}
