// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/SimpleGameComponent.hpp"

namespace CNA::Extended
{
    void SimpleGameComponent::Dispose()
    {
        if (isInitialized_)
        {
            UnloadContent();
            isInitialized_ = false;
        }
    }

    bool SimpleGameComponent::getIsEnabledProperty() const
    {
        return isEnabled_;
    }

    void SimpleGameComponent::setIsEnabledProperty(const bool value)
    {
        if (isEnabled_ == value)
        {
            return;
        }
        isEnabled_ = value;
        EnabledChanged.Raise(this, System::EventArgs::Empty);
    }

    void SimpleGameComponent::Initialize()
    {
        if (!isInitialized_)
        {
            LoadContent();
            isInitialized_ = true;
        }
    }

    void SimpleGameComponent::LoadContent()
    {
    }

    void SimpleGameComponent::UnloadContent()
    {
    }

    bool SimpleGameComponent::getEnabledProperty() const
    {
        return isEnabled_;
    }

    SharpRuntime::intcs SimpleGameComponent::getUpdateOrderProperty() const
    {
        return updateOrder_;
    }

    void SimpleGameComponent::setUpdateOrderProperty(const SharpRuntime::intcs value)
    {
        if (updateOrder_ == value)
        {
            return;
        }
        updateOrder_ = value;
        UpdateOrderChanged.Raise(this, System::EventArgs::Empty);
    }

    System::EventHandler<System::EventArgs>& SimpleGameComponent::getEnabledChangedEvent()
    {
        return EnabledChanged;
    }

    System::EventHandler<System::EventArgs>& SimpleGameComponent::getUpdateOrderChangedEvent()
    {
        return UpdateOrderChanged;
    }

    int SimpleGameComponent::CompareTo(const GameComponent& other) const
    {
        return other.getUpdateOrderProperty() - getUpdateOrderProperty();
    }

    int SimpleGameComponent::CompareTo(const SimpleGameComponent& other) const
    {
        return other.getUpdateOrderProperty() - getUpdateOrderProperty();
    }
}
