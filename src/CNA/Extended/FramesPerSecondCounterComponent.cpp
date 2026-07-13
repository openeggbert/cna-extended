// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/FramesPerSecondCounterComponent.hpp"

namespace CNA::Extended
{
    FramesPerSecondCounterComponent::FramesPerSecondCounterComponent(Game& game) : DrawableGameComponent(game)
    {
    }

    int FramesPerSecondCounterComponent::getFramesPerSecondProperty() const
    {
        return fpsCounter_.getFramesPerSecondProperty();
    }

    void FramesPerSecondCounterComponent::Update(GameTime& gameTime)
    {
        fpsCounter_.Update(gameTime);
    }

    void FramesPerSecondCounterComponent::Draw(const GameTime& gameTime)
    {
        fpsCounter_.Draw(gameTime);
    }

    const std::string& FramesPerSecondCounterComponent::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.FramesPerSecondCounterComponent";
        return typeName;
    }
}
