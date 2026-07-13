// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Input/InputListeners/MouseEventArgs.hpp"

#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"

namespace CNA::Extended::Input::InputListeners
{
    MouseEventArgs::MouseEventArgs(ViewportAdapter* viewportAdapter, const System::TimeSpan& time, const MouseState& previousState,
        const MouseState& currentState, MouseButton button)
        : Time_(time),
          PreviousState_(previousState),
          CurrentState_(currentState),
          Position_(viewportAdapter != nullptr ? viewportAdapter->PointToScreen(currentState.getXProperty(), currentState.getYProperty())
                                                : Point(currentState.getXProperty(), currentState.getYProperty())),
          Button_(button),
          ScrollWheelValue_(currentState.getScrollWheelValueProperty()),
          ScrollWheelDelta_(currentState.getScrollWheelValueProperty() - previousState.getScrollWheelValueProperty())
    {
    }

    Vector2 MouseEventArgs::getDistanceMovedProperty() const
    {
        return Vector2(static_cast<float>(CurrentState_.getXProperty()), static_cast<float>(CurrentState_.getYProperty()))
            - Vector2(static_cast<float>(PreviousState_.getXProperty()), static_cast<float>(PreviousState_.getYProperty()));
    }
}
