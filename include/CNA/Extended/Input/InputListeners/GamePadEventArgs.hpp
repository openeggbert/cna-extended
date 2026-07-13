// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/GamePadEventArgs.cs. `Buttons? button`
// -> `std::optional<Buttons>`, matching this project's nullable-value-type convention. `Vector2?
// thumbStickState` is resolved to `Vector2::Zero` when absent directly in the constructor (as
// upstream does via `thumbStickState ?? Vector2.Zero`), so the stored ThumbStickState is a plain
// Vector2, not optional -- matching upstream's own field type (`Vector2 ThumbStickState { get; }`,
// not nullable).
#pragma once

#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "System/EventArgs.hpp"
#include "System/TimeSpan.hpp"

#include <optional>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::PlayerIndex;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Input::Buttons;
    using Microsoft::Xna::Framework::Input::GamePadState;

    /** @brief Carries all data resulting from an event fired by GamePadListener. */
    class GamePadEventArgs : public System::EventArgs
    {
    public:
        /**
         * @brief Initializes a new GamePadEventArgs.
         * @param previousState The state of the controller in the previous update.
         * @param currentState The state of the controller in this update.
         * @param elapsedTime The time elapsed since the last event.
         * @param playerIndex The index of the controller.
         * @param button The button that triggered this event, if applicable.
         * @param triggerState If a TriggerMoved event, the responsible trigger's position.
         * @param thumbStickState If a ThumbStickMoved event, the responsible stick's position.
         */
        GamePadEventArgs(const GamePadState& previousState, const GamePadState& currentState, const System::TimeSpan& elapsedTime,
            PlayerIndex playerIndex, std::optional<Buttons> button = std::nullopt, float triggerState = 0.0f,
            std::optional<Vector2> thumbStickState = std::nullopt);

        /** @brief The index of the controller. */
        [[nodiscard]] PlayerIndex getPlayerIndexProperty() const { return PlayerIndex_; }

        /** @brief The state of the controller in the previous update. */
        [[nodiscard]] const GamePadState& getPreviousStateProperty() const { return PreviousState_; }

        /** @brief The state of the controller in this update. */
        [[nodiscard]] const GamePadState& getCurrentStateProperty() const { return CurrentState_; }

        /** @brief The button that triggered this event, if applicable. Default-constructed Buttons if none was given. */
        [[nodiscard]] Buttons getButtonProperty() const { return Button_; }

        /** @brief The time elapsed since the last event. */
        [[nodiscard]] const System::TimeSpan& getElapsedTimeProperty() const { return ElapsedTime_; }

        /** @brief If a TriggerMoved event, the responsible trigger's position. */
        [[nodiscard]] float getTriggerStateProperty() const { return TriggerState_; }

        /** @brief If a ThumbStickMoved event, the responsible stick's position. */
        [[nodiscard]] const Vector2& getThumbStickStateProperty() const { return ThumbStickState_; }

    private:
        PlayerIndex PlayerIndex_;
        GamePadState PreviousState_;
        GamePadState CurrentState_;
        Buttons Button_{};
        System::TimeSpan ElapsedTime_;
        float TriggerState_;
        Vector2 ThumbStickState_;
    };
}
