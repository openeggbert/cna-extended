// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/TouchListener.cs. CROSS-MODULE DEPENDENCY
// on CNA::Extended::ViewportAdapters::ViewportAdapter -- see MouseEventArgs.hpp's header comment.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListener.hpp"
#include "CNA/Extended/Input/InputListeners/TouchEventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"

namespace CNA::Extended::Input::InputListeners
{
    class TouchListenerSettings;

    /** @brief Polls the touch panel each Update() and raises started/moved/ended/cancelled events per touch location. */
    class TouchListener : public System::Object, public InputListener
    {
    public:
        System::EventHandler<TouchEventArgs> TouchStarted;
        System::EventHandler<TouchEventArgs> TouchEnded;
        System::EventHandler<TouchEventArgs> TouchMoved;
        System::EventHandler<TouchEventArgs> TouchCancelled;

        TouchListener();
        explicit TouchListener(ViewportAdapter* viewportAdapter);
        explicit TouchListener(const TouchListenerSettings& settings);

        [[nodiscard]] ViewportAdapter* getViewportAdapterProperty() const { return viewportAdapter_; }
        void setViewportAdapterProperty(ViewportAdapter* value) { viewportAdapter_ = value; }

        void Update(GameTime& gameTime) override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        ViewportAdapter* viewportAdapter_;
    };
}
