// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/RenderTarget2DExtensions.hpp"

namespace CNA::Extended::Graphics
{
    RenderTarget2DOperation::RenderTarget2DOperation(RenderTarget2D& renderTarget, GraphicsDevice& graphicsDevice, const Color& backgroundColor)
        : graphicsDevice_(&graphicsDevice),
          previousRenderTargetUsage_(graphicsDevice.getPresentationParametersProperty().getRenderTargetUsageProperty()),
          viewport_(graphicsDevice.getViewportProperty())
    {
        graphicsDevice_->getPresentationParametersProperty().setRenderTargetUsageProperty(RenderTargetUsage::PreserveContents);
        graphicsDevice_->SetRenderTarget(&renderTarget);
        graphicsDevice_->Clear(backgroundColor);
    }

    void RenderTarget2DOperation::Dispose()
    {
        graphicsDevice_->SetRenderTarget(nullptr);
        graphicsDevice_->getPresentationParametersProperty().setRenderTargetUsageProperty(previousRenderTargetUsage_);
        graphicsDevice_->setViewportProperty(viewport_);
    }

    std::unique_ptr<System::IDisposable> BeginDraw(RenderTarget2D& renderTarget, GraphicsDevice& graphicsDevice, const Color& backgroundColor)
    {
        return std::make_unique<RenderTarget2DOperation>(renderTarget, graphicsDevice, backgroundColor);
    }
}
