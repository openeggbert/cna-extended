// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/GamePadListenerSettings.cs. Property names
// preserve upstream's own "Treshold" (not "Threshold") misspelling exactly -- a real upstream
// naming quirk, not a typo introduced by this port; kept for faithful 1:1 fidelity ("port 1:1, no
// simplification" per this project's mandate applies to identifier fidelity too, not just logic).
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListenerSettings.hpp"
#include "Microsoft/Xna/Framework/PlayerIndex.hpp"

namespace CNA::Extended::Input::InputListeners
{
    class GamePadListener;

    /** @brief Settings/factory object for constructing a configured GamePadListener. */
    class GamePadListenerSettings : public InputListenerSettings<GamePadListener>
    {
    public:
        GamePadListenerSettings();

        /**
         * @brief Initializes a new GamePadListenerSettings.
         * @param playerIndex The index of the controller the listener will be tied to.
         * @param vibrationEnabled Whether vibration is enabled on the controller.
         * @param vibrationStrengthLeft General setting for the strength of the left motor.
         * @param vibrationStrengthRight General setting for the strength of the right motor.
         */
        explicit GamePadListenerSettings(Microsoft::Xna::Framework::PlayerIndex playerIndex, bool vibrationEnabled = true,
            float vibrationStrengthLeft = 1.0f, float vibrationStrengthRight = 1.0f);

        [[nodiscard]] Microsoft::Xna::Framework::PlayerIndex getPlayerIndexProperty() const { return playerIndex_; }
        void setPlayerIndexProperty(Microsoft::Xna::Framework::PlayerIndex value) { playerIndex_ = value; }

        [[nodiscard]] int getRepeatDelayProperty() const { return repeatDelay_; }
        void setRepeatDelayProperty(int value) { repeatDelay_ = value; }

        [[nodiscard]] int getRepeatInitialDelayProperty() const { return repeatInitialDelay_; }
        void setRepeatInitialDelayProperty(int value) { repeatInitialDelay_ = value; }

        [[nodiscard]] bool getVibrationEnabledProperty() const { return vibrationEnabled_; }
        void setVibrationEnabledProperty(bool value) { vibrationEnabled_ = value; }

        [[nodiscard]] float getVibrationStrengthLeftProperty() const { return vibrationStrengthLeft_; }
        void setVibrationStrengthLeftProperty(float value) { vibrationStrengthLeft_ = value; }

        [[nodiscard]] float getVibrationStrengthRightProperty() const { return vibrationStrengthRight_; }
        void setVibrationStrengthRightProperty(float value) { vibrationStrengthRight_ = value; }

        [[nodiscard]] float getTriggerDeltaTresholdProperty() const { return triggerDeltaTreshold_; }
        void setTriggerDeltaTresholdProperty(float value) { triggerDeltaTreshold_ = value; }

        [[nodiscard]] float getThumbStickDeltaTresholdProperty() const { return thumbStickDeltaTreshold_; }
        void setThumbStickDeltaTresholdProperty(float value) { thumbStickDeltaTreshold_ = value; }

        [[nodiscard]] float getTriggerDownTresholdProperty() const { return triggerDownTreshold_; }
        void setTriggerDownTresholdProperty(float value) { triggerDownTreshold_ = value; }

        [[nodiscard]] float getThumbstickDownTresholdProperty() const { return thumbstickDownTreshold_; }

        [[nodiscard]] std::unique_ptr<GamePadListener> CreateListener() const override;

    private:
        Microsoft::Xna::Framework::PlayerIndex playerIndex_;
        int repeatDelay_ = 50;
        int repeatInitialDelay_ = 500;
        bool vibrationEnabled_;
        float vibrationStrengthLeft_;
        float vibrationStrengthRight_;
        float triggerDeltaTreshold_ = 0.0f;
        float thumbStickDeltaTreshold_ = 0.0f;
        float triggerDownTreshold_ = 0.15f;
        float thumbstickDownTreshold_ = 0.5f;
    };
}
