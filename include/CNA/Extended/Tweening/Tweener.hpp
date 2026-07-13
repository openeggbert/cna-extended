// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/Tweener.cs. Manages a collection of active tween
// animations and drives their updates each frame. Create animations using TweenTo(), then call
// Update() once per frame.
//
// *** GENUINE C++ REDESIGN, NOT A LITERAL PORT -- see TweenMember.hpp/Tween.hpp for the full
// rationale ***. TweenTo's calling convention changes from a lambda member-selector
// (`tweener.TweenTo(target, x => x.Position, endValue, duration)`) to C++ pointer-to-member
// syntax (`tweener.TweenTo(&target, &TargetType::Position, endValue, duration)`).
//
// Return type: upstream's `TweenTo<TTarget,TMember>` returns `Tween<TMember>` (the GENERIC base,
// not the concrete LinearTween<T>/ColorTween subtype it actually constructs) -- ported faithfully
// as `TypedTween<TTarget,TMember>*` (the renamed equivalent, see TypedTween.hpp), not
// `LinearTween<TTarget,TMember>*`. This also cleanly unifies the Color-vs-non-Color dispatch
// below into a single template (both branches return the same TypedTween<TTarget,TMember>*),
// since C++ doesn't allow two identically-named template overloads that would otherwise be
// ambiguous for TMember=Color.
//
// Upstream's `switch (toValue) { case Color: ... }` runtime dispatch -> `if constexpr
// (std::is_same_v<TMember, Color>)` compile-time dispatch: since TMember is a template parameter
// here (unlike upstream's boxed runtime value), the Color-vs-linear choice is already known at
// compile time, so a runtime type switch is unnecessary -- another simplification the stronger
// C++ type system affords for free.
//
// Upstream's `Activator.CreateInstance(typeof(TTween), BindingFlags.Public|BindingFlags.NonPublic,
// ...)` (reflection-based construction of an arbitrary TTween subtype, needed because upstream's
// TweenTo<TTarget,TMember,TTween> lets the CALLER specify a custom TTween type by reflection) has
// no equivalent here and isn't needed: TweenTo always constructs exactly LinearTween or ColorTween
// directly via std::make_unique, since those are the only two concrete Tween kinds this port
// supports (matching what upstream's own public TweenTo<TTarget,TMember> overload -- the one
// without an explicit TTween type parameter -- actually does). Upstream's separate 3-type-parameter
// TweenTo<TTarget,TMember,TTween> overload (letting callers plug in a wholly custom TTween
// subclass) is not ported -- it exists upstream purely to make the reflection-based construction
// path extensible to third-party Tween subtypes, which has no analogue once construction is a
// direct, statically-typed std::make_unique call.
//
// Upstream's member cache (`Dictionary<TweenMemberKey, TweenMember> _memberCache`, keyed by
// (Target, MemberName) using reflection-based names) is ELIMINATED: it existed only to amortize
// the cost of reflection-based TweenMember construction, which no longer exists -- constructing a
// TweenMember<TTarget,TMember> is a trivial, cheap pointer-pair copy with nothing to cache.
//
// FindTween/the "cancel an existing tween on the same member before starting a new one" logic in
// TweenTo both use `dynamic_cast<TypedTween<TTarget,TMember>*>` to narrow each stored `Tween*` to
// the concrete type being searched for, then compare (target pointer, pointer-to-member value)
// directly with `==` -- both are genuinely comparable with `==` in standard C++, uniquely
// identifying a bound member without needing any name string (unlike upstream's
// `t.Target == target && t.MemberName == memberName` string comparison, which existed only
// because reflection gave upstream a member NAME but no cheaper identity to compare).
//
// Header-only for the template TweenTo/FindTween methods (must be visible to every calling
// translation unit, since TTarget/TMember are supplied by the caller); non-template methods
// (Update/CancelAll/CancelAndCompleteAll/Dispose) are declared here and defined in Tweener.cpp.
#pragma once

#include "CNA/Extended/Tweening/ColorTween.hpp"
#include "CNA/Extended/Tweening/LinearTween.hpp"
#include "CNA/Extended/Tweening/Tween.hpp"
#include "CNA/Extended/Tweening/TweenMember.hpp"
#include "CNA/Extended/Tweening/TypedTween.hpp"

#include <memory>
#include <type_traits>
#include <vector>

namespace CNA::Extended::Tweening
{
    /** @brief Manages a collection of active tween animations and drives their updates each frame. */
    class Tweener
    {
    public:
        Tweener() = default;

        ~Tweener() { Dispose(); }

        Tweener(const Tweener&) = delete;
        Tweener& operator=(const Tweener&) = delete;

        /** @brief Cancels all active animations and clears internal state. */
        void Dispose();

        /** @brief Gets the number of currently active tween animations. */
        [[nodiscard]] int getActiveTweensCountProperty() const { return static_cast<int>(activeTweens_.size()); }

        /**
         * @brief Creates and starts an animation that interpolates the specified member of
         * @p target from its current value to @p toValue over @p duration seconds. If an
         * animation is already running on the same (target, member) it is cancelled first.
         * @tparam TTarget The type of the target object.
         * @tparam TMember The value type of the member being animated.
         * @param target The object whose member will be animated. Must outlive the returned tween.
         * @param member Pointer to the public data member to animate.
         * @param toValue The target value to animate towards.
         * @param duration The duration of the animation in seconds.
         * @param delay An optional delay before the animation begins, in seconds.
         * @return The created tween, for fluent configuration. Owned by this Tweener; do not delete.
         */
        template <typename TTarget, typename TMember>
        TypedTween<TTarget, TMember>* TweenTo(TTarget* target, TMember TTarget::*member, TMember toValue, float duration, float delay = 0.0f)
        {
            if (TypedTween<TTarget, TMember>* existing = FindTween(target, member))
            {
                existing->Cancel();
            }

            TweenMember<TTarget, TMember> tweenMember(target, member);
            std::unique_ptr<TypedTween<TTarget, TMember>> tween;
            if constexpr (std::is_same_v<TMember, Color>)
            {
                tween = std::make_unique<ColorTween<TTarget>>(tweenMember, duration, delay, toValue);
            }
            else
            {
                tween = std::make_unique<LinearTween<TTarget, TMember>>(tweenMember, duration, delay, toValue);
            }

            TypedTween<TTarget, TMember>* result = tween.get();
            activeTweens_.push_back(std::move(tween));
            return result;
        }

        /**
         * @brief Finds and returns an active animation targeting the specified member on the
         * given object, or nullptr if no matching animation exists.
         */
        template <typename TTarget, typename TMember>
        [[nodiscard]] TypedTween<TTarget, TMember>* FindTween(TTarget* target, TMember TTarget::*member) const
        {
            for (const auto& tween : activeTweens_)
            {
                if (auto* typed = dynamic_cast<TypedTween<TTarget, TMember>*>(tween.get()))
                {
                    if (typed->getMemberProperty().getTargetProperty() == target && typed->getMemberProperty().getMemberPointerProperty() == member)
                    {
                        return typed;
                    }
                }
            }
            return nullptr;
        }

        /** @brief Advances all active animations by @p elapsedSeconds and removes any that have completed. */
        void Update(float elapsedSeconds);

        /** @brief Cancels all active animations immediately without applying their final values. */
        void CancelAll();

        /** @brief Cancels all active animations, applying the final value of each before stopping. */
        void CancelAndCompleteAll();

    private:
        std::vector<std::unique_ptr<Tween>> activeTweens_;
    };
}
