// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/TouchListenerSettings.cs. CROSS-MODULE
// DEPENDENCY on CNA::Extended::ViewportAdapters::ViewportAdapter -- see MouseEventArgs.hpp's
// header comment.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListenerSettings.hpp"

namespace CNA::Extended::ViewportAdapters
{
    class ViewportAdapter;
}

namespace CNA::Extended::Input::InputListeners
{
    class TouchListener;

    /** @brief Settings/factory object for constructing a configured TouchListener. */
    class TouchListenerSettings : public InputListenerSettings<TouchListener>
    {
    public:
        TouchListenerSettings();

        /** @brief Gets or sets the viewport adapter used to translate raw touch positions, or nullptr for raw window coordinates. */
        [[nodiscard]] CNA::Extended::ViewportAdapters::ViewportAdapter* getViewportAdapterProperty() const { return viewportAdapter_; }
        void setViewportAdapterProperty(CNA::Extended::ViewportAdapters::ViewportAdapter* value) { viewportAdapter_ = value; }

        [[nodiscard]] std::unique_ptr<TouchListener> CreateListener() const override;

    private:
        CNA::Extended::ViewportAdapters::ViewportAdapter* viewportAdapter_ = nullptr;
    };
}
