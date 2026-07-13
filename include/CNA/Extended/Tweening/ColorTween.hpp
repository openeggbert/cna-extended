// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/ColorTween.cs: a specialized tween for
// Microsoft::Xna::Framework::Color, using Color::Lerp for interpolation since a packed color
// doesn't support the plain arithmetic operators LinearTween<TTarget,TMember> relies on. Templated
// only on TTarget here (unlike upstream's non-generic `ColorTween : Tween<Color>`), since the
// value type is fixed to Color -- Tweener::TweenTo dispatches to this specialization whenever
// TMember is Color, matching upstream's own `switch (toValue) { case Color: ... }` dispatch (see
// Tweener.hpp).
//
// Header-only: a template, matching this project's established convention for generic C# types.
#pragma once

#include "CNA/Extended/Tweening/TypedTween.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

namespace CNA::Extended::Tweening
{
    using Microsoft::Xna::Framework::Color;

    /** @brief A tween that animates a Color value using Color::Lerp for interpolation. */
    template <typename TTarget>
    class ColorTween : public TypedTween<TTarget, Color>
    {
    public:
        ColorTween(const TweenMember<TTarget, Color>& member, float duration, float delay, Color endValue)
            : TypedTween<TTarget, Color>(member, duration, delay, endValue)
        {
        }

    protected:
        void Interpolate(float n) override
        {
            const Color value = Color::Lerp(this->startValue_, this->endValue_, n);
            this->getMemberProperty().setValueProperty(value);
        }
    };
}
