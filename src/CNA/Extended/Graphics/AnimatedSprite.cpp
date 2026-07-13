// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/AnimatedSprite.hpp"

#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"

#include <stdexcept>

namespace CNA::Extended::Graphics
{
    AnimatedSprite::AnimatedSprite(SpriteSheet& spriteSheet)
        : Sprite(spriteSheet.getTextureAtlasProperty()[0]), spriteSheet_(&spriteSheet)
    {
    }

    AnimatedSprite::AnimatedSprite(SpriteSheet& spriteSheet, const std::string& initialAnimation) : AnimatedSprite(spriteSheet)
    {
        animation_ = spriteSheet_->GetAnimation(initialAnimation);
        controller_ = std::make_unique<Animations::AnimationController>(*animation_);
        setTextureRegionProperty(spriteSheet_->getTextureAtlasProperty()[controller_->getCurrentFrameProperty()]);
    }

    Animations::IAnimationController& AnimatedSprite::getControllerProperty() const
    {
        if (!controller_)
        {
            throw std::logic_error("AnimatedSprite has no animation set; call SetAnimation() first.");
        }
        return *controller_;
    }

    const std::string& AnimatedSprite::getCurrentAnimationProperty() const
    {
        if (!animation_)
        {
            throw std::logic_error("AnimatedSprite has no animation set; call SetAnimation() first.");
        }
        return animation_->getNameProperty();
    }

    Animations::IAnimationController& AnimatedSprite::SetAnimation(const std::string& name)
    {
        animation_ = spriteSheet_->GetAnimation(name);
        controller_ = std::make_unique<Animations::AnimationController>(*animation_);
        setTextureRegionProperty(spriteSheet_->getTextureAtlasProperty()[controller_->getCurrentFrameProperty()]);
        return *controller_;
    }

    void AnimatedSprite::Update(const GameTime& gameTime)
    {
        Update(gameTime.getElapsedGameTimeProperty());
    }

    void AnimatedSprite::Update(const TimeSpan& elapsedTime)
    {
        if (!controller_)
        {
            throw std::logic_error("AnimatedSprite has no animation set; call SetAnimation() first.");
        }

        const int index = controller_->getCurrentFrameProperty();
        controller_->Update(elapsedTime);

        // If the current frame changed during the update, change the texture region
        if (index != controller_->getCurrentFrameProperty())
        {
            setTextureRegionProperty(spriteSheet_->getTextureAtlasProperty()[controller_->getCurrentFrameProperty()]);
        }
    }
}
