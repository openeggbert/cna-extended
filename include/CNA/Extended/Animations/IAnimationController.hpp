// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/IAnimationController.cs. Notable translation
// decisions:
//  - `event Action<IAnimationController, AnimationEventTrigger> OnAnimationEvent;` is a genuine
//    2-parameter multicast Action, not the (sender, args) EventHandler<TEventArgs> shape used
//    elsewhere in this port -- translated as System::MulticastAction<IAnimationController*,
//    AnimationEventTrigger>, exposed via a getOnAnimationEventEvent() accessor returning a
//    reference, matching the getEnabledChangedEvent()/getUpdateOrderChangedEvent() accessor
//    pattern IUpdateable already uses for interface-level events in this ecosystem (see
//    Microsoft::Xna::Framework::IUpdateable).
//  - `bool Play()` / `bool Play(int startingFrame)`, `bool Pause()` / `bool Pause(bool
//    resetFrameDuration)`, and `bool Unpause()` / `bool Unpause(bool advanceToNextFrame)` are each
//    a parameterless overload that forwards to the parameterized one with a fixed default
//    (upstream: `Play() => Play(0)`, etc.) -- collapsed into a single virtual with a default
//    argument per pair, which is behaviorally identical and more idiomatic C++ than mirroring two
//    separate pure virtuals per pair.
//  - `void Update(GameTime gameTime)` takes gameTime by value in C# but only reads
//    gameTime.ElapsedGameTime; translated as `const GameTime&` (this interface does not implement
//    IUpdateable, so it is not bound to that interface's `GameTime&` convention).
//  - `ArgumentOutOfRangeException` thrown by SetFrame/Play(int) is a concrete-implementation
//    detail (AnimationController.cpp), not representable on the interface itself.
#pragma once

#include "CNA/Extended/Animations/AnimationEventTrigger.hpp"
#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/IDisposable.hpp"
#include "System/MulticastAction.hpp"
#include "System/TimeSpan.hpp"

namespace CNA::Extended::Animations
{
    using Microsoft::Xna::Framework::GameTime;
    using System::TimeSpan;

    /** @brief Controls playback of an IAnimation: frame timing, looping/ping-pong direction, and animation events. */
    class IAnimationController : public System::IDisposable
    {
    public:
        ~IAnimationController() override = default;

        /** @brief Gets whether this controller has been disposed. */
        [[nodiscard]] virtual bool getIsDisposedProperty() const = 0;

        /** @brief Gets whether playback is currently paused. */
        [[nodiscard]] virtual bool getIsPausedProperty() const = 0;

        /** @brief Gets whether the animation is currently playing (started and not stopped). */
        [[nodiscard]] virtual bool getIsAnimatingProperty() const = 0;

        /** @brief Gets whether the animation loops when it reaches its end. */
        [[nodiscard]] virtual bool getIsLoopingProperty() const = 0;
        /** @brief Sets whether the animation loops when it reaches its end. */
        virtual void setIsLoopingProperty(bool value) = 0;

        /** @brief Gets whether the animation plays back in reverse. */
        [[nodiscard]] virtual bool getIsReversedProperty() const = 0;
        /** @brief Sets whether the animation plays back in reverse. */
        virtual void setIsReversedProperty(bool value) = 0;

        /** @brief Gets whether the animation reverses direction at each end instead of looping back to the start. */
        [[nodiscard]] virtual bool getIsPingPongProperty() const = 0;
        /** @brief Sets whether the animation reverses direction at each end instead of looping back to the start. */
        virtual void setIsPingPongProperty(bool value) = 0;

        /** @brief Gets the playback speed multiplier (1.0 is normal speed). */
        [[nodiscard]] virtual double getSpeedProperty() const = 0;
        /** @brief Sets the playback speed multiplier (1.0 is normal speed). */
        virtual void setSpeedProperty(double value) = 0;

        /** @brief Gets the multicast event raised for FrameBegin/FrameEnd/loop/completion/stop notifications. */
        [[nodiscard]] virtual System::MulticastAction<IAnimationController*, AnimationEventTrigger>& getOnAnimationEventEvent() = 0;

        /** @brief Gets the time remaining before the current frame advances. */
        [[nodiscard]] virtual TimeSpan getCurrentFrameTimeRemainingProperty() const = 0;

        /** @brief Gets the frame index (IAnimationFrame::FrameIndex) of the currently displayed frame. */
        [[nodiscard]] virtual int getCurrentFrameProperty() const = 0;

        /** @brief Gets the total number of frames in the underlying animation. */
        [[nodiscard]] virtual int getFrameCountProperty() const = 0;

        /**
         * @brief Immediately jumps to the frame at @p index and raises FrameBegin.
         * @throws System::ArgumentOutOfRangeException index is outside [0, FrameCount).
         */
        virtual void SetFrame(int index) = 0;

        /**
         * @brief Starts playback at @p startingFrame (default 0). Does nothing if already animating.
         * @return true if playback was started; false if already animating.
         * @throws System::ArgumentOutOfRangeException startingFrame is outside [0, FrameCount).
         */
        virtual bool Play(int startingFrame = 0) = 0;

        /**
         * @brief Pauses playback, optionally (@p resetFrameDuration, default false) resetting the current frame's remaining time.
         * @return true if playback was paused; false if not animating or already paused.
         */
        virtual bool Pause(bool resetFrameDuration = false) = 0;

        /**
         * @brief Resumes playback, optionally (@p advanceToNextFrame, default false) advancing to the next frame first.
         * @return true if playback was resumed; false if not animating or not paused.
         */
        virtual bool Unpause(bool advanceToNextFrame = false) = 0;

        /** @brief Advances playback by gameTime.ElapsedGameTime. */
        virtual void Update(const GameTime& gameTime) = 0;

        /** @brief Advances playback by @p elapsedTime, raising frame/loop/completion events as frames are crossed. */
        virtual void Update(TimeSpan elapsedTime) = 0;

        /**
         * @brief Stops playback and raises AnimationStopped.
         * @return true if playback was stopped; false if not animating.
         */
        virtual bool Stop() = 0;

        /** @brief Resets playback state (direction/looping/ping-pong/speed) back to the source animation's defaults. */
        virtual void Reset() = 0;
    };
}
