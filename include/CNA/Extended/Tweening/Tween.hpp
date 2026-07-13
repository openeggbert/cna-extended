// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tweening/Tween.cs. Upstream's `Tween` (non-generic) and
// `Tween<T> : Tween` (generic) cannot both be named `Tween` in C++ -- unlike C#, C++ does not
// allow a class template and a non-template class to share one name in the same scope (no arity-
// based overloading for class names). This file ports only the non-generic base (unchanged from
// upstream: none of its timing/lifecycle/easing/callback/repeat logic involves reflection). The
// generic counterpart is renamed TypedTween<TTarget, TMember> -- see TypedTween.hpp for why, and
// for the pointer-to-member redesign that replaces upstream's TweenMember<T> entirely.
//
// `Action<Tween>` callbacks (OnBegin/OnEnd/OnUpdate) -> `std::function<void(Tween&)>`, matching
// this project's established Func/Action-to-std::function convention. Upstream's `internal
// Tween(...)` constructor visibility has no C++ equivalent; kept public, matching the
// `internal`-has-no-C++-equivalent precedent set by ActorPairKey.hpp/CollisionShapeKind2D.hpp
// elsewhere in this project. `object Target` (untyped) -> `void*`, since the base class has no
// compile-time knowledge of the concrete target type (only TypedTween<TTarget,TMember> does).
// Upstream's `string MemberName` abstract property is dropped entirely: it existed only to
// support Tweener.FindTween(object, string) via reflection-free name comparison, which the
// pointer-to-member redesign replaces with a type-safe dynamic_cast + direct pointer-to-member
// comparison in Tweener::FindTween/TweenTo (see Tweener.hpp) -- no name string is ever needed.
#pragma once

#include <functional>

namespace CNA::Extended::Tweening
{
    /**
     * @brief Abstract base managing timing, lifecycle, easing, callbacks, and repeat behavior for
     * a tween animation. Subclasses (via TypedTween<TTarget,TMember>) implement the actual value
     * interpolation.
     */
    class Tween
    {
    public:
        Tween(void* target, float duration, float delay);

        virtual ~Tween() = default;

        /** @brief Gets the object whose member is being animated. */
        [[nodiscard]] void* getTargetProperty() const { return target_; }

        /** @brief Gets the total duration of one complete animation cycle, in seconds. */
        [[nodiscard]] float getDurationProperty() const { return duration_; }

        /** @brief Gets the delay before the animation begins, in seconds. */
        [[nodiscard]] float getDelayProperty() const { return delay_; }

        /** @brief Gets whether the tween is currently paused. When paused, Update() has no effect. */
        [[nodiscard]] bool getIsPausedProperty() const { return isPaused_; }
        /** @brief Sets whether the tween is currently paused. */
        void setIsPausedProperty(bool value) { isPaused_ = value; }

        /** @brief Gets whether the tween is configured to repeat after completing a cycle. */
        [[nodiscard]] bool getIsRepeatingProperty() const { return remainingRepeats_ != 0; }

        /** @brief Gets whether the tween is configured to repeat indefinitely. */
        [[nodiscard]] bool getIsRepeatingForeverProperty() const { return remainingRepeats_ < 0; }

        /** @brief Gets whether the tween reverses direction on alternate repeat cycles. */
        [[nodiscard]] bool getIsAutoReverseProperty() const { return isAutoReverse_; }

        /** @brief Gets whether the tween is active and should continue receiving updates. */
        [[nodiscard]] bool getIsAliveProperty() const { return isAlive_; }

        /** @brief Gets whether the tween has completed the current animation cycle. */
        [[nodiscard]] bool getIsCompleteProperty() const { return isComplete_; }

        /** @brief Gets the time remaining in the current animation cycle, in seconds. */
        [[nodiscard]] float getTimeRemainingProperty() const { return duration_ - elapsedDuration_; }

        /** @brief Gets the normalized progress of the current animation cycle, clamped to [0,1]. Not affected by easing. */
        [[nodiscard]] float getCompletionProperty() const;

        /** @brief Sets the easing function applied to the animation progress each frame. Returns this tween for fluent chaining. */
        Tween& Easing(std::function<float(float)> easingFunction);

        /** @brief Registers a callback invoked when the animation begins its first cycle. Returns this tween for fluent chaining. */
        Tween& OnBegin(std::function<void(Tween&)> action);

        /** @brief Registers a callback invoked when the animation completes each cycle. Returns this tween for fluent chaining. */
        Tween& OnEnd(std::function<void(Tween&)> action);

        /** @brief Registers a callback invoked after each update step, once the interpolated value has been applied. Returns this tween for fluent chaining. */
        Tween& OnUpdate(std::function<void(Tween&)> action);

        /** @brief Pauses the animation. Returns this tween for fluent chaining. */
        Tween& Pause();

        /** @brief Resumes a paused animation. Returns this tween for fluent chaining. */
        Tween& Resume();

        /** @brief Configures the animation to repeat @p count additional cycles after its first. Returns this tween for fluent chaining. */
        Tween& Repeat(int count, float repeatDelay = 0.0f);

        /** @brief Configures the animation to repeat indefinitely. Returns this tween for fluent chaining. */
        Tween& RepeatForever(float repeatDelay = 0.0f);

        /** @brief Configures the animation to reverse direction on alternate repeat cycles (ping-pong). Returns this tween for fluent chaining. */
        Tween& AutoReverse();

        /** @brief Stops the animation immediately without applying the final value. */
        void Cancel();

        /** @brief Stops the animation, applying the final value first and invoking OnEnd if still alive. */
        void CancelAndComplete();

        /** @brief Advances the animation by @p elapsedSeconds and applies the interpolated value. */
        void Update(float elapsedSeconds);

    protected:
        /** @brief Called once before the first update to capture the initial animation state. */
        virtual void Initialize() = 0;

        /** @brief Called each update to apply the interpolated value for the given eased progress @p n in [0,1]. */
        virtual void Interpolate(float n) = 0;

        /** @brief Called at the start of a reversed cycle to swap the start and end values. */
        virtual void Swap() = 0;

    private:
        void* target_;
        float duration_;
        float delay_;
        bool isPaused_ = false;
        bool isAutoReverse_ = false;
        bool isAlive_ = true;
        bool isComplete_ = false;

        std::function<float(float)> easingFunction_;
        bool isInitialized_ = false;
        float completion_ = 0.0f;
        float elapsedDuration_ = 0.0f;
        float remainingDelay_;
        float repeatDelay_ = 0.0f;
        int remainingRepeats_ = 0;
        std::function<void(Tween&)> onBegin_;
        std::function<void(Tween&)> onEnd_;
        std::function<void(Tween&)> onUpdate_;
    };
}
