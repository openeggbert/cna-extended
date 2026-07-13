// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/MouseListener.hpp"

#include "CNA/Extended/Input/InputListeners/MouseListenerSettings.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

#include <cmath>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Input::Mouse;

    MouseListener::MouseListener() : MouseListener(MouseListenerSettings())
    {
    }

    MouseListener::MouseListener(ViewportAdapter* viewportAdapter) : MouseListener(MouseListenerSettings())
    {
        viewportAdapter_ = viewportAdapter;
    }

    MouseListener::MouseListener(const MouseListenerSettings& settings)
        : viewportAdapter_(settings.getViewportAdapterProperty()),
          doubleClickMilliseconds_(settings.getDoubleClickMillisecondsProperty()),
          dragThreshold_(settings.getDragThresholdProperty())
    {
    }

    bool MouseListener::getHasMouseMovedProperty() const
    {
        return previousState_.getXProperty() != currentState_.getXProperty() || previousState_.getYProperty() != currentState_.getYProperty();
    }

    void MouseListener::CheckButtonPressed(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button)
    {
        if (getButtonState(currentState_) == ButtonState::Pressed && getButtonState(previousState_) == ButtonState::Released)
        {
            MouseEventArgs args(viewportAdapter_, gameTime_->getTotalGameTimeProperty(), previousState_, currentState_, button);

            MouseDown.Raise(this, args);
            mouseDownArgs_ = args;

            if (previousClickArgs_.has_value())
            {
                const double clickMilliseconds = (args.getTimeProperty() - previousClickArgs_->getTimeProperty()).getTotalMillisecondsProperty();

                if (clickMilliseconds <= doubleClickMilliseconds_)
                {
                    MouseDoubleClicked.Raise(this, args);
                    hasDoubleClicked_ = true;
                }

                previousClickArgs_ = std::nullopt;
            }
        }
    }

    void MouseListener::CheckButtonReleased(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button)
    {
        if (getButtonState(currentState_) == ButtonState::Released && getButtonState(previousState_) == ButtonState::Pressed)
        {
            MouseEventArgs args(viewportAdapter_, gameTime_->getTotalGameTimeProperty(), previousState_, currentState_, button);

            if (mouseDownArgs_->getButtonProperty() == args.getButtonProperty())
            {
                const int clickMovement = DistanceBetween(args.getPositionProperty(), mouseDownArgs_->getPositionProperty());

                if (clickMovement < dragThreshold_)
                {
                    if (!hasDoubleClicked_)
                    {
                        MouseClicked.Raise(this, args);
                    }
                }
                else
                {
                    MouseDragEnd.Raise(this, args);
                    dragging_ = false;
                }
            }

            MouseUp.Raise(this, args);

            hasDoubleClicked_ = false;
            previousClickArgs_ = args;
        }
    }

    void MouseListener::CheckMouseDragged(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button)
    {
        if (getButtonState(currentState_) == ButtonState::Pressed && getButtonState(previousState_) == ButtonState::Pressed)
        {
            MouseEventArgs args(viewportAdapter_, gameTime_->getTotalGameTimeProperty(), previousState_, currentState_, button);

            if (mouseDownArgs_->getButtonProperty() == args.getButtonProperty())
            {
                if (dragging_)
                {
                    MouseDrag.Raise(this, args);
                }
                else
                {
                    const int clickMovement = DistanceBetween(args.getPositionProperty(), mouseDownArgs_->getPositionProperty());

                    if (clickMovement > dragThreshold_)
                    {
                        dragging_ = true;
                        MouseDragStart.Raise(this, args);
                    }
                }
            }
        }
    }

    void MouseListener::Update(GameTime& gameTime)
    {
        gameTime_ = &gameTime;
        currentState_ = Mouse::GetState();

        CheckButtonPressed([](const MouseState& s) { return s.getLeftButtonProperty(); }, MouseButton::Left);
        CheckButtonPressed([](const MouseState& s) { return s.getMiddleButtonProperty(); }, MouseButton::Middle);
        CheckButtonPressed([](const MouseState& s) { return s.getRightButtonProperty(); }, MouseButton::Right);
        CheckButtonPressed([](const MouseState& s) { return s.getXButton1Property(); }, MouseButton::XButton1);
        CheckButtonPressed([](const MouseState& s) { return s.getXButton2Property(); }, MouseButton::XButton2);

        CheckButtonReleased([](const MouseState& s) { return s.getLeftButtonProperty(); }, MouseButton::Left);
        CheckButtonReleased([](const MouseState& s) { return s.getMiddleButtonProperty(); }, MouseButton::Middle);
        CheckButtonReleased([](const MouseState& s) { return s.getRightButtonProperty(); }, MouseButton::Right);
        CheckButtonReleased([](const MouseState& s) { return s.getXButton1Property(); }, MouseButton::XButton1);
        CheckButtonReleased([](const MouseState& s) { return s.getXButton2Property(); }, MouseButton::XButton2);

        if (getHasMouseMovedProperty())
        {
            MouseMoved.Raise(this, MouseEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), previousState_, currentState_));

            CheckMouseDragged([](const MouseState& s) { return s.getLeftButtonProperty(); }, MouseButton::Left);
            CheckMouseDragged([](const MouseState& s) { return s.getMiddleButtonProperty(); }, MouseButton::Middle);
            CheckMouseDragged([](const MouseState& s) { return s.getRightButtonProperty(); }, MouseButton::Right);
            CheckMouseDragged([](const MouseState& s) { return s.getXButton1Property(); }, MouseButton::XButton1);
            CheckMouseDragged([](const MouseState& s) { return s.getXButton2Property(); }, MouseButton::XButton2);
        }

        if (previousState_.getScrollWheelValueProperty() != currentState_.getScrollWheelValueProperty())
        {
            MouseWheelMoved.Raise(this, MouseEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), previousState_, currentState_));
        }

        previousState_ = currentState_;
    }

    int MouseListener::DistanceBetween(const Point& a, const Point& b)
    {
        return std::abs(a.X - b.X) + std::abs(a.Y - b.Y);
    }

    const std::string& MouseListener::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Input.InputListeners.MouseListener";
        return typeName;
    }
}
