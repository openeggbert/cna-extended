// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/TouchListener.hpp"

#include "CNA/Extended/Input/InputListeners/TouchListenerSettings.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Input::Touch::TouchCollection;
    using Microsoft::Xna::Framework::Input::Touch::TouchLocationState;
    using Microsoft::Xna::Framework::Input::Touch::TouchPanel;

    TouchListener::TouchListener() : TouchListener(TouchListenerSettings())
    {
    }

    TouchListener::TouchListener(ViewportAdapter* viewportAdapter) : TouchListener(TouchListenerSettings())
    {
        viewportAdapter_ = viewportAdapter;
    }

    TouchListener::TouchListener(const TouchListenerSettings& settings) : viewportAdapter_(settings.getViewportAdapterProperty())
    {
    }

    void TouchListener::Update(GameTime& gameTime)
    {
        const TouchCollection touchCollection = TouchPanel::GetState();

        for (const auto& touchLocation : touchCollection)
        {
            switch (touchLocation.getStateProperty())
            {
                case TouchLocationState::Pressed:
                    TouchStarted.Raise(this, TouchEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), touchLocation));
                    break;
                case TouchLocationState::Moved:
                    TouchMoved.Raise(this, TouchEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), touchLocation));
                    break;
                case TouchLocationState::Released:
                    TouchEnded.Raise(this, TouchEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), touchLocation));
                    break;
                case TouchLocationState::Invalid:
                    TouchCancelled.Raise(this, TouchEventArgs(viewportAdapter_, gameTime.getTotalGameTimeProperty(), touchLocation));
                    break;
            }
        }
    }

    const std::string& TouchListener::GetTypeName() const
    {
        static const std::string typeName = "MonoGame.Extended.Input.InputListeners.TouchListener";
        return typeName;
    }
}
