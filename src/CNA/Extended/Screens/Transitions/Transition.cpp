// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Screens/Transitions/Transition.hpp"

#include "CNA/Extended/GameTimeExtensions.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <stdexcept>

namespace CNA::Extended::Screens::Transitions
{
    using Microsoft::Xna::Framework::MathHelper;

    Transition::Transition(float duration) : halfDuration_(duration / 2.0f), duration_(duration)
    {
    }

    float Transition::getValueProperty() const
    {
        return MathHelper::Clamp(currentSeconds_ / halfDuration_, 0.0f, 1.0f);
    }

    void Transition::Update(GameTime& gameTime)
    {
        const float elapsedSeconds = GetElapsedSeconds(gameTime);

        switch (state_)
        {
            case TransitionState::Out:
                currentSeconds_ += elapsedSeconds;

                if (currentSeconds_ >= halfDuration_)
                {
                    state_ = TransitionState::In;
                    StateChanged.Raise(this, System::EventArgs::Empty);
                }
                break;
            case TransitionState::In:
                currentSeconds_ -= elapsedSeconds;

                if (currentSeconds_ <= 0.0f)
                {
                    Completed.Raise(this, System::EventArgs::Empty);
                }
                break;
            default:
                throw std::out_of_range("Transition::Update: unrecognized TransitionState");
        }
    }
}
