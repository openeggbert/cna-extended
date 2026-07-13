// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/TouchEventArgs.hpp"

#include "CNA/Extended/Vector2Extensions.hpp"
#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"

namespace CNA::Extended::Input::InputListeners
{
    TouchEventArgs::TouchEventArgs(ViewportAdapter* viewportAdapter, const System::TimeSpan& time, const TouchLocation& location)
        : viewportAdapter_(viewportAdapter),
          rawTouchLocation_(location),
          time_(time),
          position_(viewportAdapter != nullptr
                        ? viewportAdapter->PointToScreen(
                              static_cast<int>(location.getPositionProperty().X), static_cast<int>(location.getPositionProperty().Y))
                        : ToPoint(location.getPositionProperty()))
    {
    }

    bool TouchEventArgs::Equals(const TouchEventArgs& other) const
    {
        return this == &other || rawTouchLocation_.getIdProperty() == other.rawTouchLocation_.getIdProperty();
    }

    std::size_t TouchEventArgs::GetHashCode() const
    {
        return static_cast<std::size_t>(rawTouchLocation_.getIdProperty());
    }
}
