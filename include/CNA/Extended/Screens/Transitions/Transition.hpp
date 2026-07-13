// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Screens/Transitions/Transition.cs: an abstract base for
// screen-transition effects, driven by a half-out/half-in timing curve. Upstream lives in the
// MonoGame.Extended.Screens.Transitions namespace; ported into a matching
// CNA::Extended::Screens::Transitions sub-namespace.
//
// Only the abstract base is ported here. The two concrete subclasses (FadeTransition,
// ExpandTransition) both depend on SpriteBatch::FillRectangle (from ShapeExtensions.cs), which
// plan.md already deliberately defers in full to Phase 5 (pure SpriteBatch debug-drawing, not yet
// ported) -- see plan.md's Phase 1 scoping notes and NEXT.md for this session's entry documenting
// the deferral. Port FadeTransition/ExpandTransition once ShapeExtensions/FillRectangle lands.
#pragma once

#include "Microsoft/Xna/Framework/GameTime.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/IDisposable.hpp"
#include "System/Object.hpp"

namespace CNA::Extended::Screens::Transitions
{
    using Microsoft::Xna::Framework::GameTime;

    /** @brief Identifies whether a Transition is currently transitioning out (away from the old state) or in (towards the new state). */
    enum class TransitionState
    {
        Out,
        In
    };

    /** @brief Abstract base for a timed screen-transition effect, driven by a half-out/half-in timing curve. */
    class Transition : public System::Object, public System::IDisposable
    {
    public:
        /** @brief Raised when State changes from Out to In (the transition's midpoint). */
        System::EventHandler<System::EventArgs> StateChanged;

        /** @brief Raised once the transition has fully completed (State is In and its timer has run out). */
        System::EventHandler<System::EventArgs> Completed;

        ~Transition() override = default;

        /** @brief Releases resources used by this transition. Upstream declares this abstract (no default implementation), so this port does too. */
        void Dispose() override = 0;

        /** @brief Gets the current transition state. */
        [[nodiscard]] TransitionState getStateProperty() const { return state_; }

        /** @brief Gets the total duration of this transition, in seconds. */
        [[nodiscard]] float getDurationProperty() const { return duration_; }

        /** @brief Gets the current transition progress, in [0, 1]. */
        [[nodiscard]] float getValueProperty() const;

        /** @brief Advances the transition by GameTime::ElapsedGameTime, raising StateChanged/Completed as appropriate. */
        void Update(GameTime& gameTime);

        /** @brief Draws the transition's visual effect. */
        virtual void Draw(const GameTime& gameTime) = 0;

    protected:
        /**
         * @brief Initializes a new Transition with the specified total duration.
         * @param duration The total duration of the transition, in seconds.
         */
        explicit Transition(float duration);

    private:
        float halfDuration_;
        float currentSeconds_ = 0.0f;
        float duration_;
        TransitionState state_ = TransitionState::Out;
    };
}
