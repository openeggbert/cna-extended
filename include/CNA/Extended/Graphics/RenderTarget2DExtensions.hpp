// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/RenderTarget2DExtensions.cs. C# extension methods have
// no C++ equivalent syntax; ported as a free function in this namespace, matching this project's
// established convention. Upstream's private nested `RenderTargetOperation : IDisposable` class
// (returned only as the `IDisposable` interface type from `BeginDraw`) -> a public
// `RenderTarget2DOperation` class here, since C++ has no private-nested-type-returned-as-a-public-
// interface idiom as lightweight as C#'s -- exposing the concrete type is a visibility
// simplification only (nothing outside this file/BeginDraw() is expected to construct one
// directly), not a behavioral difference.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTarget2D.hpp"
#include "Microsoft/Xna/Framework/Graphics/RenderTargetUsage.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "System/IDisposable.hpp"

#include <memory>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::RenderTarget2D;
    using Microsoft::Xna::Framework::Graphics::RenderTargetUsage;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    /**
     * @brief RAII scope object returned by BeginDraw(): sets renderTarget active on construction,
     * restores the previous render target/RenderTargetUsage/Viewport on Dispose().
     */
    class RenderTarget2DOperation : public System::IDisposable
    {
    public:
        RenderTarget2DOperation(RenderTarget2D& renderTarget, GraphicsDevice& graphicsDevice, const Color& backgroundColor);

        void Dispose() override;

    private:
        GraphicsDevice* graphicsDevice_;
        RenderTargetUsage previousRenderTargetUsage_;
        Viewport viewport_;
    };

    /**
     * @brief Begins drawing to renderTarget: sets it active, clears it to backgroundColor, and
     * preserves its contents across subsequent render target switches until the returned scope
     * object is disposed, at which point the previous render target/state is restored.
     */
    [[nodiscard]] std::unique_ptr<System::IDisposable> BeginDraw(RenderTarget2D& renderTarget, GraphicsDevice& graphicsDevice, const Color& backgroundColor);
}
