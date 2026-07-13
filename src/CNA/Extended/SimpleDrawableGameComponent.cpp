// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/SimpleDrawableGameComponent.hpp"

namespace CNA::Extended
{
    bool SimpleDrawableGameComponent::getVisibleProperty() const
    {
        return isVisible_;
    }

    void SimpleDrawableGameComponent::setVisibleProperty(const bool value)
    {
        if (isVisible_ == value)
        {
            return;
        }
        isVisible_ = value;
        VisibleChanged.Raise(this, System::EventArgs::Empty);
    }

    SharpRuntime::intcs SimpleDrawableGameComponent::getDrawOrderProperty() const
    {
        return drawOrder_;
    }

    void SimpleDrawableGameComponent::setDrawOrderProperty(const SharpRuntime::intcs value)
    {
        if (drawOrder_ == value)
        {
            return;
        }
        drawOrder_ = value;
        DrawOrderChanged.Raise(this, System::EventArgs::Empty);
    }

    System::EventHandler<System::EventArgs>& SimpleDrawableGameComponent::getDrawOrderChangedEvent()
    {
        return DrawOrderChanged;
    }

    System::EventHandler<System::EventArgs>& SimpleDrawableGameComponent::getVisibleChangedEvent()
    {
        return VisibleChanged;
    }
}
