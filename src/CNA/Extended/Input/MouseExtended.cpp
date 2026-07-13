// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/MouseExtended.hpp"

#include "Microsoft/Xna/Framework/Input/Mouse.hpp"

namespace CNA::Extended::Input
{
    using Microsoft::Xna::Framework::Input::Mouse;

    MouseState MouseExtended::currentMouseState_;
    MouseState MouseExtended::previousMouseState_;

    MouseStateExtended MouseExtended::GetState()
    {
        return MouseStateExtended(currentMouseState_, previousMouseState_);
    }

    void MouseExtended::Update()
    {
        previousMouseState_ = currentMouseState_;
        currentMouseState_ = Mouse::GetState();
    }

    void MouseExtended::SetPosition(int x, int y)
    {
        Mouse::SetPosition(x, y);
    }

    void MouseExtended::SetPosition(const Point& point)
    {
        Mouse::SetPosition(point.X, point.Y);
    }

    void MouseExtended::SetCursor(MouseCursor& cursor)
    {
        Mouse::SetCursor(cursor);
    }

    std::uintptr_t MouseExtended::getWindowHandleProperty()
    {
        return Mouse::getWindowHandleProperty();
    }

    void MouseExtended::setWindowHandleProperty(std::uintptr_t value)
    {
        Mouse::setWindowHandleProperty(value);
    }
}
