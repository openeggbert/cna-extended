// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/LinearTween.cs. Upstream delegates arithmetic to
// LinearOperations<T> (expression-tree-compiled +/-/* delegates, working around C#'s lack of
// generic arithmetic operator constraints). LinearOperations<T> is ELIMINATED ENTIRELY in this
// port -- not ported as a separate file, deliberately -- since C++ template code can use
// operator+/operator-/operator*(T,float) directly: the compiler already enforces "T must support
// these operations" via a plain compile error for any TMember that doesn't, which is strictly
// better than C#'s runtime expression-tree compilation (a compile-time check instead of a
// first-use runtime one). Interpolate() below computes `startValue_ + range_ * n` directly.
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "CNA/Extended/Tweening/TypedTween.hpp"

namespace CNA::Extended::Tweening
{
    /**
     * @brief A tween that animates a value of type TMember using linear interpolation via TMember's
     * own operator+/operator-/operator* overloads.
     * @tparam TTarget The type of the target object.
     * @tparam TMember The value type to animate. Must support operator+(TMember,TMember),
     * operator-(TMember,TMember), and operator*(TMember,float).
     */
    template <typename TTarget, typename TMember>
    class LinearTween : public TypedTween<TTarget, TMember>
    {
    public:
        LinearTween(const TweenMember<TTarget, TMember>& member, float duration, float delay, TMember endValue)
            : TypedTween<TTarget, TMember>(member, duration, delay, endValue)
        {
        }

    protected:
        void Initialize() override
        {
            TypedTween<TTarget, TMember>::Initialize();
            range_ = this->endValue_ - this->startValue_;
        }

        void Interpolate(float n) override
        {
            const TMember value = this->startValue_ + range_ * n;
            this->getMemberProperty().setValueProperty(value);
        }

    private:
        TMember range_{};
    };
}
