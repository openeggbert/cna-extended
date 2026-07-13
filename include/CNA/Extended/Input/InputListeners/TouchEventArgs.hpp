// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Input/InputListeners/TouchEventArgs.cs. CROSS-MODULE DEPENDENCY
// on CNA::Extended::ViewportAdapters::ViewportAdapter -- see MouseEventArgs.hpp's header comment
// for the full explanation; the same nullable-raw-pointer treatment applies here.
#pragma once

#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "System/EventArgs.hpp"
#include "System/TimeSpan.hpp"

#include <cstddef>
#include <functional>

namespace CNA::Extended::ViewportAdapters
{
    class ViewportAdapter;
}

namespace CNA::Extended::Input::InputListeners
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Input::Touch::TouchLocation;
    using CNA::Extended::ViewportAdapters::ViewportAdapter;

    /** @brief Carries data for a TouchListener touch event: the raw touch location and its position translated through a ViewportAdapter, if any. */
    class TouchEventArgs : public System::EventArgs
    {
    public:
        /**
         * @brief Initializes a new TouchEventArgs.
         * @param viewportAdapter The viewport adapter to translate the raw touch position through, or nullptr to use raw window coordinates.
         * @param time The elapsed game time at which this event occurred.
         * @param location The raw touch location reported by the touch panel.
         */
        TouchEventArgs(ViewportAdapter* viewportAdapter, const System::TimeSpan& time, const TouchLocation& location);

        [[nodiscard]] ViewportAdapter* getViewportAdapterProperty() const { return viewportAdapter_; }
        [[nodiscard]] const TouchLocation& getRawTouchLocationProperty() const { return rawTouchLocation_; }
        [[nodiscard]] const System::TimeSpan& getTimeProperty() const { return time_; }
        [[nodiscard]] const Point& getPositionProperty() const { return position_; }

        [[nodiscard]] bool Equals(const TouchEventArgs& other) const;
        [[nodiscard]] std::size_t GetHashCode() const;

        friend bool operator==(const TouchEventArgs& left, const TouchEventArgs& right) { return left.Equals(right); }
        friend bool operator!=(const TouchEventArgs& left, const TouchEventArgs& right) { return !left.Equals(right); }

    private:
        ViewportAdapter* viewportAdapter_;
        TouchLocation rawTouchLocation_;
        System::TimeSpan time_;
        Point position_;
    };
}

namespace std
{
    template <>
    struct hash<CNA::Extended::Input::InputListeners::TouchEventArgs>
    {
        std::size_t operator()(const CNA::Extended::Input::InputListeners::TouchEventArgs& value) const noexcept { return value.GetHashCode(); }
    };
}
