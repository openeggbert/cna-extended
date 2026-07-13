// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/MouseListener.cs. `Func<MouseState,
// ButtonState>` private helper parameters -> `std::function<ButtonState(const MouseState&)>`.
// `_mouseDownArgs`/`_previousClickArgs` (nullable MouseEventArgs fields) -> std::optional<MouseEventArgs>.
//
// *** POSSIBLE UPSTREAM EDGE-CASE BUG, PRESERVED FOR FIDELITY, NOT FIXED ***
// CheckButtonReleased/CheckMouseDragged read `_mouseDownArgs.Button` (upstream) /
// `mouseDownArgs_->getButtonProperty()` (here) WITHOUT checking it is non-null/has_value() first.
// If a MouseUp or drag-motion frame occurs before any MouseDown was ever recorded for this
// listener (e.g. the listener is attached to an already-held button, or Update() is first called
// mid-press), upstream would throw NullReferenceException; this port has the same unchecked
// access via std::optional::operator->, which is undefined behavior on an empty optional rather
// than a clean exception -- ported as-is (not guarded), matching this project's "preserve found
// issues, document them" convention, since fixing it would silently change upstream's error
// behavior. In ordinary usage (CheckButtonPressed always runs before CheckButtonReleased/
// CheckMouseDragged for the same button within one Update()), mouseDownArgs_ is always set by the
// time these branches are reached, so this is a genuine but narrow edge case, not a
// hot-path concern.
#pragma once

#include "CNA/Extended/Input/InputListeners/InputListener.hpp"
#include "CNA/Extended/Input/InputListeners/MouseEventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"

#include <functional>
#include <optional>

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Input::ButtonState;

    class MouseListenerSettings;

    /**
     * @brief Polls mouse state each Update() and raises click/double-click/drag/move/wheel events.
     * @remark Due to the nature of the listener, it continues to be updated even when the game is
     * not in focus. Register with InputListenerComponent to avoid this, or pause Update()
     * manually when the game loses focus.
     */
    class MouseListener : public System::Object, public InputListener
    {
    public:
        System::EventHandler<MouseEventArgs> MouseDown;
        System::EventHandler<MouseEventArgs> MouseUp;
        System::EventHandler<MouseEventArgs> MouseClicked;
        System::EventHandler<MouseEventArgs> MouseDoubleClicked;
        System::EventHandler<MouseEventArgs> MouseMoved;
        System::EventHandler<MouseEventArgs> MouseWheelMoved;
        System::EventHandler<MouseEventArgs> MouseDragStart;
        System::EventHandler<MouseEventArgs> MouseDrag;
        System::EventHandler<MouseEventArgs> MouseDragEnd;

        MouseListener();
        explicit MouseListener(ViewportAdapter* viewportAdapter);
        explicit MouseListener(const MouseListenerSettings& settings);

        [[nodiscard]] ViewportAdapter* getViewportAdapterProperty() const { return viewportAdapter_; }
        [[nodiscard]] int getDoubleClickMillisecondsProperty() const { return doubleClickMilliseconds_; }
        [[nodiscard]] int getDragThresholdProperty() const { return dragThreshold_; }

        /** @brief Returns true if the mouse has moved between the current and previous frames. */
        [[nodiscard]] bool getHasMouseMovedProperty() const;

        void Update(GameTime& gameTime) override;

        [[nodiscard]] const std::string& GetTypeName() const override;

    private:
        void CheckButtonPressed(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button);
        void CheckButtonReleased(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button);
        void CheckMouseDragged(const std::function<ButtonState(const MouseState&)>& getButtonState, MouseButton button);
        [[nodiscard]] static int DistanceBetween(const Point& a, const Point& b);

        ViewportAdapter* viewportAdapter_;
        int doubleClickMilliseconds_;
        int dragThreshold_;

        MouseState currentState_;
        MouseState previousState_;
        GameTime* gameTime_ = nullptr;
        bool dragging_ = false;
        bool hasDoubleClicked_ = false;
        std::optional<MouseEventArgs> mouseDownArgs_;
        std::optional<MouseEventArgs> previousClickArgs_;
    };
}
