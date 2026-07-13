// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/AnimationEvent.cs. Upstream's constructor is
// `internal` (assembly-only visibility, no C++ equivalent) -- kept public, matching the precedent
// set by ActorPairKey elsewhere in this port. `Animation` is a non-owning reference to the
// controller that raised the event (translated as a pointer, not a C++ reference, so this type
// remains copy-assignable like a normal EventArgs-derived value); upstream itself performs no
// null check in this constructor, so none is added here.
//
// NOTE: as of the upstream revision this was ported from, AnimationController itself never
// constructs an AnimationEvent -- it raises OnAnimationEvent as the raw
// Action<IAnimationController, AnimationEventTrigger> directly (see AnimationController.cpp).
// AnimationEvent appears to be a currently-unused-within-this-module convenience type; ported
// as-is since it is part of the module's public surface, not silently dropped.
#pragma once

#include "CNA/Extended/Animations/AnimationEventTrigger.hpp"
#include "System/EventArgs.hpp"

namespace CNA::Extended::Animations
{
    class IAnimationController;

    /** @brief Event data describing an animation-controller notification (see AnimationEventTrigger). */
    class AnimationEvent : public System::EventArgs
    {
    public:
        /** @brief Creates event data for @p trigger raised by @p animation. */
        AnimationEvent(IAnimationController* animation, AnimationEventTrigger trigger);

        /** @brief Gets the controller that raised this event. */
        [[nodiscard]] IAnimationController* getAnimationProperty() const { return animation_; }

        /** @brief Gets the kind of event that was raised. */
        [[nodiscard]] AnimationEventTrigger getTriggerProperty() const { return trigger_; }

    private:
        IAnimationController* animation_;
        AnimationEventTrigger trigger_;
    };
}
