// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/EasingFunctions.cs: a collection of standard easing
// functions for animation/tweening. No reflection or expression trees involved (unlike the rest of
// this module -- see Tween.hpp/TweenMember.hpp for the pointer-to-member redesign rationale) --
// these are plain static float-to-float functions, ported directly. C#'s `Func<float,float>` ->
// `std::function<float(float)>`, matching this project's established Func/Action-to-std::function
// convention.
#pragma once

#include <functional>

namespace CNA::Extended::Tweening
{
    /** @brief Standard easing functions for animation/tweening; each maps a normalized [0,1] progress value to a transformed one. */
    class EasingFunctions final
    {
    public:
        EasingFunctions() = delete;

        /** @brief Returns a new easing function that is the inverse of @p easer, mirroring its curve ("In" <-> "Out"). */
        [[nodiscard]] static std::function<float(float)> Invert(std::function<float(float)> easer);

        /** @brief Returns a new easing function applying @p first for progress in [0,0.5] and @p second for (0.5,1]. */
        [[nodiscard]] static std::function<float(float)> Follow(std::function<float(float)> first, std::function<float(float)> second);

        /** @brief A linear easing function with a constant rate of change. */
        [[nodiscard]] static float Linear(float value);

        [[nodiscard]] static float CubicIn(float value);
        [[nodiscard]] static float CubicOut(float value);
        [[nodiscard]] static float CubicInOut(float value);

        [[nodiscard]] static float QuadraticIn(float value);
        [[nodiscard]] static float QuadraticOut(float value);
        [[nodiscard]] static float QuadraticInOut(float value);

        [[nodiscard]] static float QuarticIn(float value);
        [[nodiscard]] static float QuarticOut(float value);
        [[nodiscard]] static float QuarticInOut(float value);

        [[nodiscard]] static float QuinticIn(float value);
        [[nodiscard]] static float QuinticOut(float value);
        [[nodiscard]] static float QuinticInOut(float value);

        [[nodiscard]] static float SineIn(float value);
        [[nodiscard]] static float SineOut(float value);
        [[nodiscard]] static float SineInOut(float value);

        [[nodiscard]] static float ExponentialIn(float value);
        [[nodiscard]] static float ExponentialOut(float value);
        [[nodiscard]] static float ExponentialInOut(float value);

        [[nodiscard]] static float CircleIn(float value);
        [[nodiscard]] static float CircleOut(float value);
        [[nodiscard]] static float CircleInOut(float value);

        [[nodiscard]] static float ElasticIn(float value);
        [[nodiscard]] static float ElasticOut(float value);
        [[nodiscard]] static float ElasticInOut(float value);

        [[nodiscard]] static float BackIn(float value);
        [[nodiscard]] static float BackOut(float value);
        [[nodiscard]] static float BackInOut(float value);

        [[nodiscard]] static float BounceIn(float value);
        [[nodiscard]] static float BounceOut(float value);
        [[nodiscard]] static float BounceInOut(float value);

    private:
        [[nodiscard]] static float Out(float value, const std::function<float(float)>& function);
        [[nodiscard]] static float InOut(float value, const std::function<float(float)>& function);

        struct Power
        {
            [[nodiscard]] static float In(double value, int power);
            [[nodiscard]] static float Out(double value, int power);
            [[nodiscard]] static float InOut(double s, int power);
        };
    };
}
