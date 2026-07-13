// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/KeyboardListener.hpp"

#include "CNA/Extended/Input/InputListeners/KeyboardListenerSettings.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"

#include <array>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Input::Keyboard;

    namespace
    {
        // Every Keys enumerator, in the exact order declared in Keys.hpp -- the C++ equivalent of
        // upstream's `Enum.GetValues(typeof(Keys))`, which C++ has no reflection to derive automatically.
        constexpr std::array<Keys, 160> kAllKeys{
            Keys::None, Keys::Back, Keys::Tab, Keys::Enter, Keys::Pause, Keys::CapsLock, Keys::Kana, Keys::Kanji, Keys::Escape,
            Keys::ImeConvert, Keys::ImeNoConvert, Keys::Space, Keys::PageUp, Keys::PageDown, Keys::End, Keys::Home, Keys::Left,
            Keys::Up, Keys::Right, Keys::Down, Keys::Select, Keys::Print, Keys::Execute, Keys::PrintScreen, Keys::Insert,
            Keys::Delete, Keys::Help, Keys::D0, Keys::D1, Keys::D2, Keys::D3, Keys::D4, Keys::D5, Keys::D6, Keys::D7, Keys::D8,
            Keys::D9, Keys::A, Keys::B, Keys::C, Keys::D, Keys::E, Keys::F, Keys::G, Keys::H, Keys::I, Keys::J, Keys::K, Keys::L,
            Keys::M, Keys::N, Keys::O, Keys::P, Keys::Q, Keys::R, Keys::S, Keys::T, Keys::U, Keys::V, Keys::W, Keys::X, Keys::Y,
            Keys::Z, Keys::LeftWindows, Keys::RightWindows, Keys::Apps, Keys::Sleep, Keys::NumPad0, Keys::NumPad1, Keys::NumPad2,
            Keys::NumPad3, Keys::NumPad4, Keys::NumPad5, Keys::NumPad6, Keys::NumPad7, Keys::NumPad8, Keys::NumPad9,
            Keys::Multiply, Keys::Add, Keys::Separator, Keys::Subtract, Keys::Decimal, Keys::Divide, Keys::F1, Keys::F2, Keys::F3,
            Keys::F4, Keys::F5, Keys::F6, Keys::F7, Keys::F8, Keys::F9, Keys::F10, Keys::F11, Keys::F12, Keys::F13, Keys::F14,
            Keys::F15, Keys::F16, Keys::F17, Keys::F18, Keys::F19, Keys::F20, Keys::F21, Keys::F22, Keys::F23, Keys::F24,
            Keys::NumLock, Keys::Scroll, Keys::LeftShift, Keys::RightShift, Keys::LeftControl, Keys::RightControl, Keys::LeftAlt,
            Keys::RightAlt, Keys::BrowserBack, Keys::BrowserForward, Keys::BrowserRefresh, Keys::BrowserStop, Keys::BrowserSearch,
            Keys::BrowserFavorites, Keys::BrowserHome, Keys::VolumeMute, Keys::VolumeDown, Keys::VolumeUp, Keys::MediaNextTrack,
            Keys::MediaPreviousTrack, Keys::MediaStop, Keys::MediaPlayPause, Keys::LaunchMail, Keys::SelectMedia,
            Keys::LaunchApplication1, Keys::LaunchApplication2, Keys::OemSemicolon, Keys::OemPlus, Keys::OemComma, Keys::OemMinus,
            Keys::OemPeriod, Keys::OemQuestion, Keys::OemTilde, Keys::ChatPadGreen, Keys::ChatPadOrange, Keys::OemOpenBrackets,
            Keys::OemPipe, Keys::OemCloseBrackets, Keys::OemQuotes, Keys::Oem8, Keys::OemBackslash, Keys::ProcessKey,
            Keys::OemCopy, Keys::OemAuto, Keys::OemEnlW, Keys::Attn, Keys::Crsel, Keys::Exsel, Keys::EraseEof, Keys::Play,
            Keys::Zoom, Keys::Pa1, Keys::OemClear};
    }

    KeyboardListener::KeyboardListener() : KeyboardListener(KeyboardListenerSettings())
    {
    }

    KeyboardListener::KeyboardListener(const KeyboardListenerSettings& settings)
        : repeatPress_(settings.getRepeatPressProperty()),
          initialDelay_(settings.getInitialDelayMillisecondsProperty()),
          repeatDelay_(settings.getRepeatDelayMillisecondsProperty())
    {
    }

    void KeyboardListener::Update(GameTime& gameTime)
    {
        const KeyboardState currentState = Keyboard::GetState();

        RaisePressedEvents(gameTime, currentState);
        RaiseReleasedEvents(currentState);

        if (repeatPress_)
        {
            RaiseRepeatEvents(gameTime, currentState);
        }

        previousState_ = currentState;
    }

    void KeyboardListener::RaisePressedEvents(GameTime& gameTime, const KeyboardState& currentState)
    {
        if (!currentState.IsKeyDown(Keys::LeftAlt) && !currentState.IsKeyDown(Keys::RightAlt))
        {
            for (Keys key : kAllKeys)
            {
                if (!(currentState.IsKeyDown(key) && previousState_.IsKeyUp(key)))
                {
                    continue;
                }

                KeyboardEventArgs args(key, currentState);

                KeyPressed.Raise(this, args);

                if (args.getCharacterProperty().has_value())
                {
                    KeyTyped.Raise(this, args);
                }

                previousKey_ = key;
                lastPressTime_ = gameTime.getTotalGameTimeProperty();
                isInitial_ = true;
            }
        }
    }

    void KeyboardListener::RaiseReleasedEvents(const KeyboardState& currentState)
    {
        for (Keys key : kAllKeys)
        {
            if (currentState.IsKeyUp(key) && previousState_.IsKeyDown(key))
            {
                KeyboardEventArgs args(key, currentState);
                KeyReleased.Raise(this, args);
            }
        }
    }

    void KeyboardListener::RaiseRepeatEvents(GameTime& gameTime, const KeyboardState& currentState)
    {
        const double elapsedTime = (gameTime.getTotalGameTimeProperty() - lastPressTime_).getTotalMillisecondsProperty();

        if (currentState.IsKeyDown(previousKey_) && ((isInitial_ && elapsedTime > initialDelay_) || (!isInitial_ && elapsedTime > repeatDelay_)))
        {
            KeyboardEventArgs args(previousKey_, currentState);

            KeyPressed.Raise(this, args);

            if (args.getCharacterProperty().has_value())
            {
                KeyTyped.Raise(this, args);
            }

            lastPressTime_ = gameTime.getTotalGameTimeProperty();
            isInitial_ = false;
        }
    }

    const std::string& KeyboardListener::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Input.InputListeners.KeyboardListener";
        return typeName;
    }
}
