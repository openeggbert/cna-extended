// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/Tween.cs's generic `Tween<T> : Tween` class -- renamed
// TypedTween<TTarget, TMember> because C++ does not allow a class template and a non-template
// class to share one name in the same scope, unlike C#'s arity-based generic overloading (see
// Tween.hpp's header comment). Uses TweenMember<TTarget, TMember> (pointer-to-member based) in
// place of upstream's reflection-based TweenMember<T> -- see TweenMember.hpp for the full redesign
// rationale.
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "CNA/Extended/Tweening/Tween.hpp"
#include "CNA/Extended/Tweening/TweenMember.hpp"

namespace CNA::Extended::Tweening
{
    /**
     * @brief Abstract base for a typed tween that animates a value of type TMember on a target
     * object's member, identified via pointer-to-member.
     * @tparam TTarget The type of the target object.
     * @tparam TMember The value type being animated.
     */
    template <typename TTarget, typename TMember>
    class TypedTween : public Tween
    {
    public:
        /**
         * @brief Initializes a new instance targeting the specified member.
         * @param member The bound target object and pointer-to-member to animate.
         * @param duration The total duration of one complete animation cycle, in seconds.
         * @param delay The delay before the animation begins, in seconds.
         * @param endValue The target value to animate towards.
         */
        TypedTween(const TweenMember<TTarget, TMember>& member, float duration, float delay, TMember endValue)
            : Tween(member.getTargetProperty(), duration, delay), startValue_(endValue), endValue_(endValue), member_(member)
        {
        }

        /** @brief Gets the TweenMember representing the property or field being animated. */
        [[nodiscard]] const TweenMember<TTarget, TMember>& getMemberProperty() const { return member_; }

    protected:
        /**
         * @brief The value of the member at the start of the current animation cycle.
         * @remark Constructor-initialized to endValue as a placeholder (not `TMember{}`): TMember
         * is not guaranteed default-constructible (e.g. Microsoft::Xna::Framework::Color has no
         * default constructor). The placeholder is always overwritten by Initialize() before any
         * real interpolation reads it.
         */
        TMember startValue_;

        /** @brief The target value to animate towards. */
        TMember endValue_;

        void Initialize() override { startValue_ = member_.getValueProperty(); }

        void Swap() override
        {
            endValue_ = startValue_;
            Initialize();
        }

    private:
        TweenMember<TTarget, TMember> member_;
    };
}
