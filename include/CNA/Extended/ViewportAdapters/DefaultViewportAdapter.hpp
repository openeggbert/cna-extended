// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ViewportAdapters/DefaultViewportAdapter.cs: a pass-through
// adapter with no scaling, exposing the graphics device's actual viewport as-is.
#pragma once

#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"

#include <string>

namespace CNA::Extended::ViewportAdapters
{
    /** @brief A pass-through ViewportAdapter with no scaling; VirtualWidth/Height always match the actual viewport. */
    class DefaultViewportAdapter : public ViewportAdapter
    {
    public:
        explicit DefaultViewportAdapter(GraphicsDevice& graphicsDevice) : ViewportAdapter(graphicsDevice) {}

        [[nodiscard]] int getVirtualWidthProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getWidthProperty(); }
        [[nodiscard]] int getVirtualHeightProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getHeightProperty(); }
        [[nodiscard]] int getViewportWidthProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getWidthProperty(); }
        [[nodiscard]] int getViewportHeightProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getHeightProperty(); }

        [[nodiscard]] Matrix GetScaleMatrix() const override { return Matrix::getIdentityProperty(); }

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.ViewportAdapters.DefaultViewportAdapter";
            return typeName;
        }
    };
}
