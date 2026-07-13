// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/MouseListenerSettings.cs. CROSS-MODULE
// DEPENDENCY on CNA::Extended::ViewportAdapters::ViewportAdapter -- see MouseEventArgs.hpp's
// header comment for the full explanation. ViewportAdapter is not owned by this settings object
// (a caller-supplied, externally-owned adapter, matching upstream's reference-type field), so it
// is stored as a non-owning raw pointer.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListenerSettings.hpp"

namespace CNA::Extended::ViewportAdapters
{
    class ViewportAdapter;
}

namespace CNA::Extended::Input::InputListeners
{
    class MouseListener;

    /** @brief Settings/factory object for constructing a configured MouseListener. */
    class MouseListenerSettings : public InputListenerSettings<MouseListener>
    {
    public:
        MouseListenerSettings();

        /** @brief Gets or sets the minimum pointer movement, in pixels, that distinguishes a click from a drag. */
        [[nodiscard]] int getDragThresholdProperty() const { return dragThreshold_; }
        void setDragThresholdProperty(int value) { dragThreshold_ = value; }

        /** @brief Gets or sets the maximum interval, in milliseconds, between two clicks to count as a double-click. */
        [[nodiscard]] int getDoubleClickMillisecondsProperty() const { return doubleClickMilliseconds_; }
        void setDoubleClickMillisecondsProperty(int value) { doubleClickMilliseconds_ = value; }

        /** @brief Gets or sets the viewport adapter used to translate raw mouse positions, or nullptr for raw window coordinates. */
        [[nodiscard]] CNA::Extended::ViewportAdapters::ViewportAdapter* getViewportAdapterProperty() const { return viewportAdapter_; }
        void setViewportAdapterProperty(CNA::Extended::ViewportAdapters::ViewportAdapter* value) { viewportAdapter_ = value; }

        [[nodiscard]] std::unique_ptr<MouseListener> CreateListener() const override;

    private:
        // initial values are windows defaults
        int doubleClickMilliseconds_ = 500;
        int dragThreshold_ = 2;
        CNA::Extended::ViewportAdapters::ViewportAdapter* viewportAdapter_ = nullptr;
    };
}
