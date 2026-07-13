// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ViewportAdapters/WindowViewportAdapter.cs: an adapter whose
// virtual resolution always tracks the window's client area exactly (no scaling), updated live via
// GameWindow::ClientSizeChanged. GameWindow is taken by reference, matching ViewportAdapter's own
// GraphicsDevice& convention -- the adapter never owns the window. Upstream never unsubscribes
// from ClientSizeChanged (only BoxingViewportAdapter's Dispose() does, for its own subscription) --
// preserved exactly, not "fixed": if a WindowViewportAdapter outlives its subscription's usefulness
// but the window itself outlives the adapter, the dangling subscription is upstream's own
// behavior, not introduced by this port.
#pragma once

#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

#include <string>

namespace CNA::Extended::ViewportAdapters
{
    using Microsoft::Xna::Framework::GameWindow;

    /** @brief A ViewportAdapter whose virtual resolution always tracks the window's client area exactly. */
    class WindowViewportAdapter : public ViewportAdapter
    {
    public:
        WindowViewportAdapter(GameWindow& window, GraphicsDevice& graphicsDevice);

        [[nodiscard]] int getViewportWidthProperty() const override { return Window.getClientBoundsProperty().Width; }
        [[nodiscard]] int getViewportHeightProperty() const override { return Window.getClientBoundsProperty().Height; }
        [[nodiscard]] int getVirtualWidthProperty() const override { return Window.getClientBoundsProperty().Width; }
        [[nodiscard]] int getVirtualHeightProperty() const override { return Window.getClientBoundsProperty().Height; }

        [[nodiscard]] Matrix GetScaleMatrix() const override { return Matrix::getIdentityProperty(); }

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.ViewportAdapters.WindowViewportAdapter";
            return typeName;
        }

    protected:
        GameWindow& Window;

    private:
        void OnClientSizeChanged(System::Object* sender, const System::EventArgs& eventArgs);
    };
}
