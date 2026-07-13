// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/GamePadListener.hpp"

#include "CNA/Extended/Input/InputListeners/GamePadListenerSettings.hpp"
#include "Microsoft/Xna/Framework/Input/GamePad.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::PlayerIndex;
    using Microsoft::Xna::Framework::Input::GamePad;

    namespace
    {
        // Every Buttons enumerator CNA defines, in ascending numeric (flag-bit) order -- the C++
        // equivalent of upstream's `Enum.GetValues(typeof(Buttons))`. Includes CNA's FNA-extension
        // values (Misc1EXT/Paddle1-4EXT/TouchPadEXT), which upstream's real XNA Buttons enum does
        // not define -- see this file's header comment for why this doesn't change the excluded-
        // buttons break/continue behavior.
        constexpr std::array<Buttons, 31> kAllButtons{Buttons::DPadUp, Buttons::DPadDown, Buttons::DPadLeft, Buttons::DPadRight,
            Buttons::Start, Buttons::Back, Buttons::LeftStick, Buttons::RightStick, Buttons::LeftShoulder, Buttons::RightShoulder,
            Buttons::Misc1EXT, Buttons::BigButton, Buttons::A, Buttons::B, Buttons::X, Buttons::Y, Buttons::Paddle1EXT,
            Buttons::Paddle2EXT, Buttons::Paddle3EXT, Buttons::Paddle4EXT, Buttons::TouchPadEXT, Buttons::LeftThumbstickLeft,
            Buttons::RightTrigger, Buttons::LeftTrigger, Buttons::RightThumbstickUp, Buttons::RightThumbstickDown,
            Buttons::RightThumbstickRight, Buttons::RightThumbstickLeft, Buttons::LeftThumbstickUp, Buttons::LeftThumbstickDown,
            Buttons::LeftThumbstickRight};

        // These buttons are not evaluated by CheckAllButtons's simple down/up scan, but with the
        // debounce filter in CheckTriggers/CheckThumbSticks instead.
        constexpr std::array<Buttons, 10> kExcludedButtons{Buttons::LeftTrigger, Buttons::RightTrigger, Buttons::LeftThumbstickDown,
            Buttons::LeftThumbstickUp, Buttons::LeftThumbstickRight, Buttons::LeftThumbstickLeft, Buttons::RightThumbstickLeft,
            Buttons::RightThumbstickRight, Buttons::RightThumbstickUp, Buttons::RightThumbstickDown};

        bool IsExcludedButton(Buttons button)
        {
            for (Buttons excluded : kExcludedButtons)
            {
                if (excluded == button)
                {
                    return true;
                }
            }
            return false;
        }
    }

    bool GamePadListener::CheckControllerConnectionsProperty = false;
    System::EventHandler<GamePadEventArgs> GamePadListener::ControllerConnectionChanged;
    std::array<bool, 4> GamePadListener::gamePadConnections_{};

    GamePadListener::GamePadListener() : GamePadListener(GamePadListenerSettings())
    {
    }

    GamePadListener::GamePadListener(const GamePadListenerSettings& settings)
        : playerIndex_(settings.getPlayerIndexProperty()),
          repeatDelay_(settings.getRepeatDelayProperty()),
          repeatInitialDelay_(settings.getRepeatInitialDelayProperty()),
          vibrationEnabled_(settings.getVibrationEnabledProperty()),
          triggerDeltaTreshold_(settings.getTriggerDeltaTresholdProperty()),
          thumbStickDeltaTreshold_(settings.getThumbStickDeltaTresholdProperty()),
          triggerDownTreshold_(settings.getTriggerDownTresholdProperty()),
          thumbstickDownTreshold_(settings.getThumbstickDownTresholdProperty())
    {
        setVibrationStrengthLeftProperty(settings.getVibrationStrengthLeftProperty());
        setVibrationStrengthRightProperty(settings.getVibrationStrengthRightProperty());
    }

    void GamePadListener::setVibrationStrengthLeftProperty(float value)
    {
        vibrationStrengthLeft_ = MathHelper::Clamp(value, 0.0f, 1.0f);
    }

    void GamePadListener::setVibrationStrengthRightProperty(float value)
    {
        vibrationStrengthRight_ = MathHelper::Clamp(value, 0.0f, 1.0f);
    }

    bool GamePadListener::Vibrate(int durationMs, float leftStrength, float rightStrength)
    {
        if (!vibrationEnabled_)
        {
            return false;
        }

        float lstrength = MathHelper::Clamp(leftStrength, 0.0f, 1.0f);
        float rstrength = MathHelper::Clamp(rightStrength, 0.0f, 1.0f);

        const bool leftIsNegativeInfinity = leftStrength == -std::numeric_limits<float>::infinity();
        const bool rightIsNegativeInfinity = rightStrength == -std::numeric_limits<float>::infinity();

        if (leftIsNegativeInfinity)
        {
            lstrength = leftCurVibrationStrength_;
        }
        if (rightIsNegativeInfinity)
        {
            rstrength = rightCurVibrationStrength_;
        }

        const bool success = GamePad::SetVibration(playerIndex_, lstrength * vibrationStrengthLeft_, rstrength * vibrationStrengthRight_);
        if (success)
        {
            leftVibrating_ = true;
            rightVibrating_ = true;

            if (leftStrength > 0)
            {
                vibrationDurationLeft_ = System::TimeSpan(0, 0, 0, 0, durationMs);
            }
            else
            {
                if (lstrength > 0)
                {
                    vibrationDurationLeft_ = vibrationDurationLeft_ - (gameTime_->getTotalGameTimeProperty() - vibrationStart_);
                }
                else
                {
                    leftVibrating_ = false;
                }
            }

            if (rightStrength > 0)
            {
                vibrationDurationRight_ = System::TimeSpan(0, 0, 0, 0, durationMs);
            }
            else
            {
                if (rstrength > 0)
                {
                    vibrationDurationRight_ = vibrationDurationRight_ - (gameTime_->getTotalGameTimeProperty() - vibrationStart_);
                }
                else
                {
                    rightVibrating_ = false;
                }
            }

            vibrationStart_ = gameTime_->getTotalGameTimeProperty();

            leftCurVibrationStrength_ = lstrength;
            rightCurVibrationStrength_ = rstrength;
        }
        return success;
    }

    void GamePadListener::CheckAllButtons()
    {
        for (Buttons button : kAllButtons)
        {
            if (IsExcludedButton(button))
            {
                break;
            }
            if (currentState_.IsButtonDown(button) && previousState_.IsButtonUp(button))
            {
                RaiseButtonDown(button);
            }
            if (currentState_.IsButtonUp(button) && previousState_.IsButtonDown(button))
            {
                RaiseButtonUp(button);
            }
        }

        CheckTriggers([](const GamePadState& s) { return s.getTriggersProperty().getLeftProperty(); }, Buttons::LeftTrigger);
        CheckTriggers([](const GamePadState& s) { return s.getTriggersProperty().getRightProperty(); }, Buttons::RightTrigger);

        CheckThumbSticks([](const GamePadState& s) { return s.getThumbSticksProperty().getRightProperty(); }, Buttons::RightStick);
        CheckThumbSticks([](const GamePadState& s) { return s.getThumbSticksProperty().getLeftProperty(); }, Buttons::LeftStick);
    }

    void GamePadListener::CheckTriggers(const std::function<float(const GamePadState&)>& getButtonState, Buttons button)
    {
        constexpr float debounce = 0.05f;
        const float curstate = getButtonState(currentState_);
        const bool curdown = curstate > triggerDownTreshold_;
        const bool prevdown = button == Buttons::RightTrigger ? rightTriggerDown_ : leftTriggerDown_;

        if (!prevdown && curdown)
        {
            RaiseButtonDown(button);
            if (button == Buttons::RightTrigger)
            {
                rightTriggerDown_ = true;
            }
            else
            {
                leftTriggerDown_ = true;
            }
        }
        else
        {
            if (prevdown && (curstate < debounce))
            {
                RaiseButtonUp(button);
                if (button == Buttons::RightTrigger)
                {
                    rightTriggerDown_ = false;
                }
                else
                {
                    leftTriggerDown_ = false;
                }
            }
        }

        const float prevstate = getButtonState(lastTriggerState_);
        if (curstate > triggerDeltaTreshold_)
        {
            if (std::abs(prevstate - curstate) >= triggerDeltaTreshold_)
            {
                TriggerMoved.Raise(this, MakeArgs(button, curstate));
                lastTriggerState_ = currentState_;
            }
        }
        else
        {
            if (prevstate > triggerDeltaTreshold_)
            {
                TriggerMoved.Raise(this, MakeArgs(button, curstate));
                lastTriggerState_ = currentState_;
            }
        }
    }

    void GamePadListener::CheckThumbSticks(const std::function<Vector2(const GamePadState&)>& getButtonState, Buttons button)
    {
        constexpr float debounce = 0.15f;
        const Vector2 curVector = getButtonState(currentState_);
        const bool curdown = curVector.Length() > thumbstickDownTreshold_;
        const bool right = button == Buttons::RightStick;
        const bool prevdown = right ? rightStickDown_ : leftStickDown_;

        const Buttons prevdir = button == Buttons::RightStick ? lastRightStickDirection_ : lastLeftStickDirection_;
        Buttons curdir;
        if (curVector.Y > curVector.X)
        {
            curdir = curVector.Y > -curVector.X ? (right ? Buttons::RightThumbstickUp : Buttons::LeftThumbstickUp)
                                                 : (right ? Buttons::RightThumbstickLeft : Buttons::LeftThumbstickLeft);
        }
        else
        {
            curdir = curVector.Y < -curVector.X ? (right ? Buttons::RightThumbstickDown : Buttons::LeftThumbstickDown)
                                                 : (right ? Buttons::RightThumbstickRight : Buttons::LeftThumbstickRight);
        }

        if (!prevdown && curdown)
        {
            if (right)
            {
                lastRightStickDirection_ = curdir;
            }
            else
            {
                lastLeftStickDirection_ = curdir;
            }

            RaiseButtonDown(curdir);
            if (button == Buttons::RightStick)
            {
                rightStickDown_ = true;
            }
            else
            {
                leftStickDown_ = true;
            }
        }
        else
        {
            if (prevdown && (curVector.Length() < debounce))
            {
                RaiseButtonUp(prevdir);
                if (button == Buttons::RightStick)
                {
                    rightStickDown_ = false;
                }
                else
                {
                    leftStickDown_ = false;
                }
            }
            else
            {
                if (prevdown && curdown && (curdir != prevdir))
                {
                    RaiseButtonUp(prevdir);
                    if (right)
                    {
                        lastRightStickDirection_ = curdir;
                    }
                    else
                    {
                        lastLeftStickDirection_ = curdir;
                    }
                    RaiseButtonDown(curdir);
                }
            }
        }

        const Vector2 prevVector = getButtonState(lastThumbStickState_);
        if (curVector.Length() > thumbStickDeltaTreshold_)
        {
            if (Vector2::Distance(curVector, prevVector) >= thumbStickDeltaTreshold_)
            {
                ThumbStickMoved.Raise(this, MakeArgs(button, 0.0f, curVector));
                lastThumbStickState_ = currentState_;
            }
        }
        else
        {
            if (prevVector.Length() > thumbStickDeltaTreshold_)
            {
                ThumbStickMoved.Raise(this, MakeArgs(button, 0.0f, curVector));
                lastThumbStickState_ = currentState_;
            }
        }
    }

    void GamePadListener::CheckConnections()
    {
        if (!CheckControllerConnectionsProperty)
        {
            return;
        }

        constexpr std::array<PlayerIndex, 4> kAllPlayerIndexes{PlayerIndex::One, PlayerIndex::Two, PlayerIndex::Three, PlayerIndex::Four};

        for (PlayerIndex index : kAllPlayerIndexes)
        {
            const int i = static_cast<int>(index);
            if (GamePad::GetState(index).getIsConnectedProperty() != gamePadConnections_[static_cast<std::size_t>(i)])
            {
                gamePadConnections_[static_cast<std::size_t>(i)] = !gamePadConnections_[static_cast<std::size_t>(i)];
                ControllerConnectionChanged.Raise(
                    nullptr, GamePadEventArgs(GamePadState(), GamePad::GetState(index), System::TimeSpan::Zero, index));
            }
        }
    }

    void GamePadListener::CheckVibrate()
    {
        if (leftVibrating_ && (vibrationStart_ + vibrationDurationLeft_ < gameTime_->getTotalGameTimeProperty()))
        {
            Vibrate(0, 0.0f);
        }
        if (rightVibrating_ && (vibrationStart_ + vibrationDurationRight_ < gameTime_->getTotalGameTimeProperty()))
        {
            Vibrate(0, -std::numeric_limits<float>::infinity(), 0.0f);
        }
    }

    void GamePadListener::Update(GameTime& gameTime)
    {
        gameTime_ = &gameTime;
        currentState_ = GamePad::GetState(playerIndex_);
        CheckVibrate();
        if (!currentState_.getIsConnectedProperty())
        {
            return;
        }
        CheckAllButtons();
        CheckRepeatButton();
        previousGameTime_ = gameTime;
        previousState_ = currentState_;
    }

    GamePadEventArgs GamePadListener::MakeArgs(std::optional<Buttons> button, float triggerState, std::optional<Vector2> thumbStickState) const
    {
        const System::TimeSpan elapsedTime = gameTime_->getTotalGameTimeProperty() - previousGameTime_.getTotalGameTimeProperty();
        return {previousState_, currentState_, elapsedTime, playerIndex_, button, triggerState, thumbStickState};
    }

    void GamePadListener::RaiseButtonDown(Buttons button)
    {
        ButtonDown.Raise(this, MakeArgs(button));
        ButtonRepeated.Raise(this, MakeArgs(button));
        lastButton_ = button;
        repeatedButtonTimer_ = 0;
    }

    void GamePadListener::RaiseButtonUp(Buttons button)
    {
        ButtonUp.Raise(this, MakeArgs(button));
        lastButton_ = static_cast<Buttons>(0);
    }

    void GamePadListener::CheckRepeatButton()
    {
        repeatedButtonTimer_ += gameTime_->getElapsedGameTimeProperty().getMillisecondsProperty();

        if ((repeatedButtonTimer_ < repeatInitialDelay_) || (lastButton_ == static_cast<Buttons>(0)))
        {
            return;
        }

        if (repeatedButtonTimer_ < repeatInitialDelay_ + repeatDelay_)
        {
            ButtonRepeated.Raise(this, MakeArgs(lastButton_));
            repeatedButtonTimer_ = repeatDelay_ + repeatInitialDelay_;
        }
        else
        {
            if (repeatedButtonTimer_ > repeatInitialDelay_ + repeatDelay_ * 2)
            {
                ButtonRepeated.Raise(this, MakeArgs(lastButton_));
                repeatedButtonTimer_ = repeatDelay_ + repeatInitialDelay_;
            }
        }
    }

    const std::string& GamePadListener::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Input.InputListeners.GamePadListener";
        return typeName;
    }
}
