// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/FramesPerSecondCounter.hpp"

namespace CNA::Extended
{
    using System::TimeSpan;

    FramesPerSecondCounter::FramesPerSecondCounter() : timer_(TimeSpan(0, 0, 1))
    {
    }

    bool FramesPerSecondCounter::getEnabledProperty() const
    {
        return enabled_;
    }

    void FramesPerSecondCounter::setEnabledProperty(const bool value)
    {
        if (enabled_ == value)
        {
            return;
        }
        enabled_ = value;
        EnabledChanged.Raise(this, System::EventArgs::Empty);
    }

    SharpRuntime::intcs FramesPerSecondCounter::getUpdateOrderProperty() const
    {
        return updateOrder_;
    }

    void FramesPerSecondCounter::setUpdateOrderProperty(const SharpRuntime::intcs value)
    {
        if (updateOrder_ == value)
        {
            return;
        }
        updateOrder_ = value;
        // Matches upstream: raises EnabledChanged, not UpdateOrderChanged. See header comment.
        EnabledChanged.Raise(this, System::EventArgs::Empty);
    }

    System::EventHandler<System::EventArgs>& FramesPerSecondCounter::getEnabledChangedEvent()
    {
        return EnabledChanged;
    }

    System::EventHandler<System::EventArgs>& FramesPerSecondCounter::getUpdateOrderChangedEvent()
    {
        return UpdateOrderChanged;
    }

    int FramesPerSecondCounter::getFramesPerSecondProperty() const
    {
        return framesPerSecond_;
    }

    void FramesPerSecondCounter::Update(GameTime& gameTime)
    {
        const TimeSpan oneSecond(0, 0, 1);

        timer_ = timer_ + gameTime.getElapsedGameTimeProperty();
        if (timer_ <= oneSecond)
        {
            return;
        }

        framesPerSecond_ = framesCounter_;
        framesCounter_ = 0;
        timer_ = timer_ - oneSecond;
    }

    void FramesPerSecondCounter::Draw(const GameTime&)
    {
        framesCounter_++;
    }

    const std::string& FramesPerSecondCounter::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.FramesPerSecondCounter";
        return typeName;
    }
}
