// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Animations/AnimationController.cs. Notable translation
// decisions:
//  - The constructor takes `IAnimation& definition` (a reference, so upstream's implicit
//    non-null contract is enforced by the type system with no explicit null check needed,
//    matching ActorPairKey's precedent elsewhere in this port) but stores it internally as
//    `IAnimation* definition_` rather than a reference member, so AnimationController itself
//    keeps ordinary (if not copy-assignable in practice, still relocatable-by-pointer) value
//    semantics matching how AnimationComponent stores controllers as non-owning pointers.
//  - `Play()`/`Pause()`/`Unpause()` collapse their C# parameterless-overload-forwards-to-default
//    pattern into a single method with a default argument (see IAnimationController.hpp).
//  - The private `Stop(AnimationEventTrigger)` overload is kept with upstream's exact name
//    (distinguished from the public `Stop()` by arity only, exactly as upstream distinguishes
//    them by C# overload resolution).
//  - `GC.SuppressFinalize(this)` in upstream's public `Dispose()` has no C++ equivalent (no GC)
//    and is dropped; `Dispose(bool disposing)`'s `disposing` parameter is unused by upstream's own
//    base implementation (reserved for subclasses to override), so it is explicitly discarded via
//    `(void)disposing;`, matching GameComponent::Dispose's established idiom for unused override
//    parameters elsewhere in this ecosystem.
//  - TimeSpan has no compound-assignment operators (no operator-=) in sharp-runtime, so
//    `CurrentFrameTimeRemaining -= x` is translated as `currentFrameTimeRemaining_ =
//    currentFrameTimeRemaining_ - x` throughout Update(TimeSpan).
#pragma once

#include "CNA/Extended/Animations/IAnimationController.hpp"
#include "CNA/Extended/Animations/IAnimation.hpp"

namespace CNA::Extended::Animations
{
    /** @brief Default IAnimationController implementation: frame-timing, looping/ping-pong, and event playback for an IAnimation. */
    class AnimationController : public IAnimationController
    {
    public:
        /** @brief Creates a controller for @p definition, seeded from its IsLooping/IsReversed/IsPingPong defaults, and starts playback at frame 0. */
        explicit AnimationController(IAnimation& definition);

        ~AnimationController() override = default;

        [[nodiscard]] bool getIsDisposedProperty() const override { return isDisposed_; }
        [[nodiscard]] bool getIsPausedProperty() const override { return isPaused_; }
        [[nodiscard]] bool getIsAnimatingProperty() const override { return isAnimating_; }

        [[nodiscard]] bool getIsLoopingProperty() const override { return isLooping_; }
        void setIsLoopingProperty(bool value) override { isLooping_ = value; }

        [[nodiscard]] bool getIsReversedProperty() const override { return direction_ == -1; }
        void setIsReversedProperty(bool value) override { direction_ = value ? -1 : 1; }

        [[nodiscard]] bool getIsPingPongProperty() const override { return isPingPong_; }
        void setIsPingPongProperty(bool value) override { isPingPong_ = value; }

        [[nodiscard]] double getSpeedProperty() const override { return speed_; }
        void setSpeedProperty(double value) override { speed_ = value; }

        [[nodiscard]] System::MulticastAction<IAnimationController*, AnimationEventTrigger>& getOnAnimationEventEvent() override { return onAnimationEvent_; }

        [[nodiscard]] TimeSpan getCurrentFrameTimeRemainingProperty() const override { return currentFrameTimeRemaining_; }
        [[nodiscard]] int getCurrentFrameProperty() const override;
        [[nodiscard]] int getFrameCountProperty() const override;

        void SetFrame(int index) override;

        bool Play(int startingFrame = 0) override;
        bool Pause(bool resetFrameDuration = false) override;
        bool Unpause(bool advanceToNextFrame = false) override;

        void Update(const GameTime& gameTime) override;
        void Update(TimeSpan elapsedTime) override;

        bool Stop() override;
        void Reset() override;

        void Dispose() override;

    protected:
        /** @brief Standard dispose-pattern hook; upstream's own base implementation does not use @p disposing (reserved for subclasses). */
        virtual void Dispose(bool disposing);

    private:
        /** @brief Stops playback, raising @p trigger instead of the default AnimationStopped (used internally by AdvanceFrame on natural completion). */
        bool Stop(AnimationEventTrigger trigger);

        /** @brief Advances to the next frame, applying looping/ping-pong wraparound; returns false (and stops) if a non-looping animation ran out of frames. */
        [[nodiscard]] bool AdvanceFrame();

        IAnimation* definition_;
        int direction_;
        int internalFrame_;
        bool isDisposed_ = false;
        bool isPaused_ = false;
        bool isAnimating_ = false;
        bool isLooping_;
        bool isPingPong_;
        double speed_;
        System::MulticastAction<IAnimationController*, AnimationEventTrigger> onAnimationEvent_;
        TimeSpan currentFrameTimeRemaining_;
    };
}
