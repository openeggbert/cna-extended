// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ViewportAdapters/ScalingViewportAdapter.cs: an adapter with a
// fixed virtual resolution, uniformly non-uniformly scaled (independent X/Y factors) to fill the
// actual viewport.
#pragma once

#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"

#include <string>

namespace CNA::Extended::ViewportAdapters
{
    /** @brief A ViewportAdapter with a fixed virtual resolution, scaled (independently in X/Y) to fill the actual viewport. */
    class ScalingViewportAdapter : public ViewportAdapter
    {
    public:
        ScalingViewportAdapter(GraphicsDevice& graphicsDevice, int virtualWidth, int virtualHeight)
            : ViewportAdapter(graphicsDevice), virtualWidth_(virtualWidth), virtualHeight_(virtualHeight)
        {
        }

        [[nodiscard]] int getVirtualWidthProperty() const override { return virtualWidth_; }
        [[nodiscard]] int getVirtualHeightProperty() const override { return virtualHeight_; }
        [[nodiscard]] int getViewportWidthProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getWidthProperty(); }
        [[nodiscard]] int getViewportHeightProperty() const override { return getGraphicsDeviceProperty().getViewportProperty().getHeightProperty(); }

        [[nodiscard]] Matrix GetScaleMatrix() const override;

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.ViewportAdapters.ScalingViewportAdapter";
            return typeName;
        }

    private:
        int virtualWidth_;
        int virtualHeight_;
    };
}
