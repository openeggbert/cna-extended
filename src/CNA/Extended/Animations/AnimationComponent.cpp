// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Animations/AnimationComponent.hpp"

#include <algorithm>

namespace CNA::Extended::Animations
{
    AnimationComponent::AnimationComponent(Game& game) : GameComponent(game)
    {
    }

    void AnimationComponent::Update(GameTime& gameTime)
    {
        GameComponent::Update(gameTime);

        for (auto it = animations_.rbegin(); it != animations_.rend(); ++it)
        {
            (*it)->Update(gameTime);
        }

        animations_.erase(
            std::remove_if(animations_.begin(), animations_.end(),
                [](const AnimationController* animation) { return animation->getIsDisposedProperty(); }),
            animations_.end());
    }

    const std::string& AnimationComponent::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Animations.AnimationComponent";
        return typeName;
    }
}
