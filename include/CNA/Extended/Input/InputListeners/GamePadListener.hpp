// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/GamePadListener.cs. `Enum.GetValues(typeof(Buttons))`
// has no C++ reflection equivalent -- ported as a static const std::array<Buttons, 31> literal in
// the .cpp, listing every Buttons enumerator CNA defines (including its FNA-extension values
// Misc1EXT/Paddle1-4EXT/TouchPadEXT, which upstream's real XNA Buttons enum does not have -- a
// genuine, unavoidable set-size difference between the two languages' Buttons enums, not a
// fidelity gap in the port itself), ordered by ascending numeric flag value to match .NET's
// observed Enum.GetValues ordering for flags enums.
//
// *** SUSPICIOUS CONTROL FLOW FOUND, INVESTIGATED, PRESERVED AS-IS (empirically harmless here) ***
// Upstream's CheckAllButtons() does `if (_excludedButtons.Contains(button)) break;` inside the
// all-buttons loop -- `break`, not `continue`. A `break` here would, in general, wrongly abort
// the whole loop on the FIRST excluded button rather than skipping just that one. Investigated by
// checking the actual numeric ordering of every Buttons value: all 10 excluded buttons
// (LeftTrigger/RightTrigger/*ThumbstickUp/Down/Left/Right) happen to be exactly the buttons with
// the highest flag bits, clustered contiguously at the end of ascending-numeric order, with no
// non-excluded button positioned after them. So breaking on the first excluded button reached
// (in ascending order) never actually skips a legitimate button -- everything remaining past that
// point is excluded anyway. Ported as `break` (not silently "fixed" to `continue`), since fixing
// it would diverge from upstream even though the observable behavior happens to be identical
// either way for this specific enum layout; if CNA's Buttons enum values are ever renumbered,
// this coincidence would stop holding and the bug would start to matter.
#pragma once

#include "CNA/Extended/Input/InputListeners/GamePadEventArgs.hpp"
#include "CNA/Extended/Input/InputListeners/InputListener.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"

#include <array>
#include <functional>
#include <limits>
#include <optional>

namespace CNA::Extended::Input::InputListeners
{
    class GamePadListenerSettings;

    /** @brief Polls one controller's state each Update() and raises button/trigger/thumbstick events. */
    class GamePadListener : public System::Object, public InputListener
    {
    public:
        /**
         * @brief If set to true, ControllerConnectionChanged fires when any controller changes
         * connectivity status. Requires an actively-updating GamePadListener.
         */
        static bool CheckControllerConnectionsProperty;

        /** @brief Raised whenever a controller connects or disconnects, if CheckControllerConnectionsProperty is true. */
        static System::EventHandler<GamePadEventArgs> ControllerConnectionChanged;

        /** @brief Raised whenever a button changes from the Up to the Down state. */
        System::EventHandler<GamePadEventArgs> ButtonDown;

        /** @brief Raised whenever a button changes from the Down to the Up state. */
        System::EventHandler<GamePadEventArgs> ButtonUp;

        /** @brief Raised repeatedly whenever a button is held sufficiently long. */
        System::EventHandler<GamePadEventArgs> ButtonRepeated;

        /** @brief Raised whenever a thumbstick changes position beyond ThumbStickDeltaTreshold. */
        System::EventHandler<GamePadEventArgs> ThumbStickMoved;

        /** @brief Raised whenever a trigger changes position beyond TriggerDeltaTreshold. */
        System::EventHandler<GamePadEventArgs> TriggerMoved;

        GamePadListener();
        explicit GamePadListener(const GamePadListenerSettings& settings);

        [[nodiscard]] Microsoft::Xna::Framework::PlayerIndex getPlayerIndexProperty() const { return playerIndex_; }
        [[nodiscard]] int getRepeatDelayProperty() const { return repeatDelay_; }
        [[nodiscard]] int getRepeatInitialDelayProperty() const { return repeatInitialDelay_; }

        [[nodiscard]] bool getVibrationEnabledProperty() const { return vibrationEnabled_; }
        void setVibrationEnabledProperty(bool value) { vibrationEnabled_ = value; }

        [[nodiscard]] float getVibrationStrengthLeftProperty() const { return vibrationStrengthLeft_; }
        void setVibrationStrengthLeftProperty(float value);

        [[nodiscard]] float getVibrationStrengthRightProperty() const { return vibrationStrengthRight_; }
        void setVibrationStrengthRightProperty(float value);

        [[nodiscard]] float getTriggerDeltaTresholdProperty() const { return triggerDeltaTreshold_; }
        [[nodiscard]] float getThumbStickDeltaTresholdProperty() const { return thumbStickDeltaTreshold_; }
        [[nodiscard]] float getTriggerDownTresholdProperty() const { return triggerDownTreshold_; }
        [[nodiscard]] float getThumbstickDownTresholdProperty() const { return thumbstickDownTreshold_; }

        /**
         * @brief Sends a vibration command to the controller.
         * @param durationMs Duration of the vibration in milliseconds.
         * @param leftStrength The strength of the left motor, or a negative-infinity sentinel to preserve its current strength.
         * @param rightStrength The strength of the right motor, or a negative-infinity sentinel to preserve its current strength.
         * @return true if the operation succeeded.
         */
        bool Vibrate(int durationMs, float leftStrength = -std::numeric_limits<float>::infinity(),
            float rightStrength = -std::numeric_limits<float>::infinity());

        void Update(GameTime& gameTime) override;

        /** @brief Polls every player index's connection state and raises ControllerConnectionChanged for any change, if enabled. */
        static void CheckConnections();

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        void CheckAllButtons();
        void CheckTriggers(const std::function<float(const GamePadState&)>& getTriggerState, Buttons button);
        void CheckThumbSticks(const std::function<Vector2(const GamePadState&)>& getThumbStickState, Buttons button);
        void CheckVibrate();
        [[nodiscard]] GamePadEventArgs MakeArgs(std::optional<Buttons> button, float triggerState = 0.0f,
            std::optional<Vector2> thumbStickState = std::nullopt) const;
        void RaiseButtonDown(Buttons button);
        void RaiseButtonUp(Buttons button);
        void CheckRepeatButton();

        static std::array<bool, 4> gamePadConnections_;

        Microsoft::Xna::Framework::PlayerIndex playerIndex_;
        int repeatDelay_;
        int repeatInitialDelay_;
        bool vibrationEnabled_;
        float vibrationStrengthLeft_ = 0.0f;
        float vibrationStrengthRight_ = 0.0f;
        float triggerDeltaTreshold_;
        float thumbStickDeltaTreshold_;
        float triggerDownTreshold_;
        float thumbstickDownTreshold_;

        GamePadState currentState_;
        GamePadState previousState_;
        GameTime* gameTime_ = nullptr;
        GameTime previousGameTime_;

        Buttons lastButton_{};
        Buttons lastLeftStickDirection_{};
        Buttons lastRightStickDirection_{};
        GamePadState lastThumbStickState_;
        GamePadState lastTriggerState_;

        float leftCurVibrationStrength_ = 0.0f;
        float rightCurVibrationStrength_ = 0.0f;
        bool leftStickDown_ = false;
        bool rightStickDown_ = false;
        bool leftTriggerDown_ = false;
        bool rightTriggerDown_ = false;
        bool leftVibrating_ = false;
        bool rightVibrating_ = false;
        System::TimeSpan vibrationDurationLeft_;
        System::TimeSpan vibrationDurationRight_;
        System::TimeSpan vibrationStart_;
        int repeatedButtonTimer_ = 0;
    };
}
