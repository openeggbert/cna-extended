// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/MouseEventArgs.cs. CROSS-MODULE
// DEPENDENCY: upstream depends on MonoGame.Extended.ViewportAdapters.ViewportAdapter (a Phase 3
// module being ported separately, in parallel with this one) for its `viewportAdapter?.
// PointToScreen(...)` call. ViewportAdapter is forward-declared here and used only via a pointer
// parameter (upstream's `viewportAdapter?.` null-conditional -> nullable ViewportAdapter*, unlike
// this project's usual const-ref-for-non-nullable-parameter convention, since null genuinely means
// something here: "no viewport adapter configured, use raw window coordinates directly").
// Calling `viewportAdapter->PointToScreen(int, int)` returning a `Point` assumes that method's
// exact upstream signature -- verify this compiles once CNA::Extended::ViewportAdapters::
// ViewportAdapter lands with its real API; this file was written without being able to read that
// type's finished header, only its C# source (`ViewportAdapter.PointToScreen(int x, int y)`
// returning `Point`, per the upstream ViewportAdapters/ViewportAdapter.cs source read alongside
// this file).
#pragma once

#include "CNA/Extended/Input/MouseButton.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/EventArgs.hpp"
#include "System/TimeSpan.hpp"

namespace CNA::Extended::ViewportAdapters
{
    class ViewportAdapter;
}

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Input::MouseState;
    using CNA::Extended::ViewportAdapters::ViewportAdapter;

    /** @brief Carries data for a MouseListener mouse event: position, button, and scroll-wheel state at the time of the event. */
    class MouseEventArgs : public System::EventArgs
    {
    public:
        /**
         * @brief Initializes a new MouseEventArgs.
         * @param viewportAdapter The viewport adapter to translate the raw mouse position through, or nullptr to use raw window coordinates.
         * @param time The elapsed game time at which this event occurred.
         * @param previousState The mouse state during the previous update cycle.
         * @param currentState The mouse state during the current update cycle.
         * @param button The button responsible for this event, or MouseButton::None if not button-specific.
         */
        MouseEventArgs(ViewportAdapter* viewportAdapter, const System::TimeSpan& time, const MouseState& previousState,
            const MouseState& currentState, MouseButton button = MouseButton::None);

        [[nodiscard]] const System::TimeSpan& getTimeProperty() const { return Time_; }
        [[nodiscard]] const MouseState& getPreviousStateProperty() const { return PreviousState_; }
        [[nodiscard]] const MouseState& getCurrentStateProperty() const { return CurrentState_; }
        [[nodiscard]] const Point& getPositionProperty() const { return Position_; }
        [[nodiscard]] MouseButton getButtonProperty() const { return Button_; }
        [[nodiscard]] int getScrollWheelValueProperty() const { return ScrollWheelValue_; }
        [[nodiscard]] int getScrollWheelDeltaProperty() const { return ScrollWheelDelta_; }

        /** @brief Gets the distance the mouse has moved between the previous and current state, in window coordinates. */
        [[nodiscard]] Vector2 getDistanceMovedProperty() const;

    private:
        System::TimeSpan Time_;
        MouseState PreviousState_;
        MouseState CurrentState_;
        Point Position_;
        MouseButton Button_;
        int ScrollWheelValue_;
        int ScrollWheelDelta_;
    };
}
