// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Animations/AnimationController.hpp"

#include "System/ArgumentOutOfRangeException.hpp"

namespace CNA::Extended::Animations
{
    AnimationController::AnimationController(IAnimation& definition)
        : definition_(&definition),
          direction_(definition.getIsReversedProperty() ? -1 : 1),
          internalFrame_(0),
          isLooping_(definition.getIsLoopingProperty()),
          isPingPong_(definition.getIsPingPongProperty()),
          speed_(1.0),
          currentFrameTimeRemaining_(TimeSpan::Zero)
    {
        Play();
    }

    int AnimationController::getCurrentFrameProperty() const
    {
        return definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getFrameIndexProperty();
    }

    int AnimationController::getFrameCountProperty() const
    {
        return definition_->getFrameCountProperty();
    }

    void AnimationController::SetFrame(int index)
    {
        System::ArgumentOutOfRangeException::ThrowIfLessThan(index, 0, "index");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(index, definition_->getFrameCountProperty(), "index");

        internalFrame_ = index;
        currentFrameTimeRemaining_ = definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getDurationProperty();
        onAnimationEvent_(this, AnimationEventTrigger::FrameBegin);
    }

    bool AnimationController::Play(int startingFrame)
    {
        System::ArgumentOutOfRangeException::ThrowIfLessThan(startingFrame, 0, "startingFrame");
        System::ArgumentOutOfRangeException::ThrowIfGreaterThanOrEqual(startingFrame, definition_->getFrameCountProperty(), "startingFrame");

        if (isAnimating_)
        {
            return false;
        }

        isAnimating_ = true;
        internalFrame_ = startingFrame;
        currentFrameTimeRemaining_ = definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getDurationProperty();
        return true;
    }

    bool AnimationController::Pause(bool resetFrameDuration)
    {
        if (!isAnimating_ || isPaused_)
        {
            return false;
        }

        isPaused_ = true;
        if (resetFrameDuration)
        {
            currentFrameTimeRemaining_ = definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getDurationProperty();
        }
        return true;
    }

    bool AnimationController::Unpause(bool advanceToNextFrame)
    {
        if (!isAnimating_ || !isPaused_)
        {
            return false;
        }

        isPaused_ = false;
        if (advanceToNextFrame)
        {
            (void)AdvanceFrame();
        }
        return true;
    }

    void AnimationController::Update(const GameTime& gameTime)
    {
        Update(gameTime.getElapsedGameTimeProperty());
    }

    void AnimationController::Update(TimeSpan elapsedTime)
    {
        if (!isAnimating_ || isPaused_)
        {
            return;
        }

        TimeSpan remainingTime = TimeSpan::Zero;
        currentFrameTimeRemaining_ = currentFrameTimeRemaining_ - elapsedTime * speed_;
        while (currentFrameTimeRemaining_ <= TimeSpan::Zero)
        {
            remainingTime = remainingTime + (-currentFrameTimeRemaining_);
            onAnimationEvent_(this, AnimationEventTrigger::FrameEnd);
            if (!AdvanceFrame())
            {
                break;
            }
            currentFrameTimeRemaining_ = currentFrameTimeRemaining_ - remainingTime;
            remainingTime = TimeSpan::Zero;
        }
    }

    bool AnimationController::Stop()
    {
        return Stop(AnimationEventTrigger::AnimationStopped);
    }

    bool AnimationController::Stop(AnimationEventTrigger trigger)
    {
        if (!isAnimating_)
        {
            return false;
        }

        isAnimating_ = false;
        isPaused_ = true;
        onAnimationEvent_(this, trigger);
        return true;
    }

    void AnimationController::Reset()
    {
        direction_ = definition_->getIsReversedProperty() ? -1 : 1;
        isPingPong_ = definition_->getIsPingPongProperty();
        isLooping_ = definition_->getIsLoopingProperty();
        isAnimating_ = false;
        isPaused_ = true;
        speed_ = 1.0;
        internalFrame_ = (direction_ == -1) ? definition_->getFrameCountProperty() - 1 : 0;
        currentFrameTimeRemaining_ = definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getDurationProperty();
    }

    bool AnimationController::AdvanceFrame()
    {
        internalFrame_ += direction_;
        const int frameCount = definition_->getFrameCountProperty();
        if (internalFrame_ < 0 || internalFrame_ >= frameCount)
        {
            if (isLooping_)
            {
                if (isPingPong_)
                {
                    direction_ = -direction_;
                    internalFrame_ += direction_ * 2;
                }
                else
                {
                    internalFrame_ = (direction_ == -1) ? frameCount - 1 : 0;
                }
                onAnimationEvent_(this, AnimationEventTrigger::AnimationLoop);
            }
            else
            {
                internalFrame_ -= direction_;
                Stop(AnimationEventTrigger::AnimationCompleted);
                return false;
            }
        }

        currentFrameTimeRemaining_ = definition_->getFramesProperty()[static_cast<std::size_t>(internalFrame_)]->getDurationProperty();
        onAnimationEvent_(this, AnimationEventTrigger::FrameBegin);
        return true;
    }

    void AnimationController::Dispose()
    {
        Dispose(true);
    }

    void AnimationController::Dispose(bool disposing)
    {
        (void)disposing;
        if (isDisposed_)
        {
            return;
        }
        isDisposed_ = true;
    }
}
