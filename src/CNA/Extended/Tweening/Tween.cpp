// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tweening/Tween.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

namespace CNA::Extended::Tweening
{
    using Microsoft::Xna::Framework::MathHelper;

    Tween::Tween(void* target, float duration, float delay) : target_(target), duration_(duration), delay_(delay), remainingDelay_(delay)
    {
    }

    float Tween::getCompletionProperty() const
    {
        return MathHelper::Clamp(completion_, 0.0f, 1.0f);
    }

    Tween& Tween::Easing(std::function<float(float)> easingFunction)
    {
        easingFunction_ = std::move(easingFunction);
        return *this;
    }

    Tween& Tween::OnBegin(std::function<void(Tween&)> action)
    {
        onBegin_ = std::move(action);
        return *this;
    }

    Tween& Tween::OnEnd(std::function<void(Tween&)> action)
    {
        onEnd_ = std::move(action);
        return *this;
    }

    Tween& Tween::OnUpdate(std::function<void(Tween&)> action)
    {
        onUpdate_ = std::move(action);
        return *this;
    }

    Tween& Tween::Pause()
    {
        isPaused_ = true;
        return *this;
    }

    Tween& Tween::Resume()
    {
        isPaused_ = false;
        return *this;
    }

    Tween& Tween::Repeat(int count, float repeatDelay)
    {
        remainingRepeats_ = count;
        repeatDelay_ = repeatDelay;
        return *this;
    }

    Tween& Tween::RepeatForever(float repeatDelay)
    {
        remainingRepeats_ = -1;
        repeatDelay_ = repeatDelay;
        return *this;
    }

    Tween& Tween::AutoReverse()
    {
        if (remainingRepeats_ == 0)
        {
            remainingRepeats_ = 1;
        }

        isAutoReverse_ = true;
        return *this;
    }

    void Tween::Cancel()
    {
        remainingRepeats_ = 0;
        isAlive_ = false;
    }

    void Tween::CancelAndComplete()
    {
        if (isAlive_)
        {
            completion_ = 1.0f;

            Interpolate(1.0f);
            isComplete_ = true;
            if (onEnd_)
            {
                onEnd_(*this);
            }
        }

        Cancel();
    }

    void Tween::Update(float elapsedSeconds)
    {
        if (isPaused_ || !isAlive_)
        {
            return;
        }

        if (remainingDelay_ > 0.0f)
        {
            remainingDelay_ -= elapsedSeconds;

            if (remainingDelay_ > 0.0f)
            {
                return;
            }
        }

        if (!isInitialized_)
        {
            isInitialized_ = true;
            Initialize();
            if (onBegin_)
            {
                onBegin_(*this);
            }
        }

        if (isComplete_)
        {
            isComplete_ = false;
            elapsedDuration_ = 0.0f;
            if (onBegin_)
            {
                onBegin_(*this);
            }

            if (isAutoReverse_)
            {
                Swap();
            }
        }

        elapsedDuration_ += elapsedSeconds;

        float n = completion_ = elapsedDuration_ / duration_;

        if (easingFunction_)
        {
            n = easingFunction_(n);
        }

        if (elapsedDuration_ >= duration_)
        {
            if (remainingRepeats_ != 0)
            {
                if (remainingRepeats_ > 0)
                {
                    remainingRepeats_--;
                }

                remainingDelay_ = repeatDelay_;
            }
            else
            {
                isAlive_ = false;
            }

            n = completion_ = 1.0f;
            isComplete_ = true;
        }

        Interpolate(n);
        if (onUpdate_)
        {
            onUpdate_(*this);
        }

        if (isComplete_)
        {
            if (onEnd_)
            {
                onEnd_(*this);
            }
        }
    }
}
