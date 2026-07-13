// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Animations/AnimationEvent.hpp"

namespace CNA::Extended::Animations
{
    AnimationEvent::AnimationEvent(IAnimationController* animation, AnimationEventTrigger trigger)
        : animation_(animation), trigger_(trigger)
    {
    }
}
