// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ViewportAdapters/ScalingViewportAdapter.hpp"

namespace CNA::Extended::ViewportAdapters
{
    Matrix ScalingViewportAdapter::GetScaleMatrix() const
    {
        const float scaleX = static_cast<float>(getViewportWidthProperty()) / static_cast<float>(virtualWidth_);
        const float scaleY = static_cast<float>(getViewportHeightProperty()) / static_cast<float>(virtualHeight_);
        return Matrix::CreateScale(scaleX, scaleY, 1.0f);
    }
}
