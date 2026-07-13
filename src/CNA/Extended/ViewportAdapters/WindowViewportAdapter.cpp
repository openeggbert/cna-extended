// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ViewportAdapters/WindowViewportAdapter.hpp"

#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"

namespace CNA::Extended::ViewportAdapters
{
    WindowViewportAdapter::WindowViewportAdapter(GameWindow& window, GraphicsDevice& graphicsDevice) : ViewportAdapter(graphicsDevice), Window(window)
    {
        Window.ClientSizeChanged += [this](System::Object* sender, const System::EventArgs& eventArgs) { OnClientSizeChanged(sender, eventArgs); };
    }

    void WindowViewportAdapter::OnClientSizeChanged(System::Object*, const System::EventArgs&)
    {
        const int x = Window.getClientBoundsProperty().Width;
        const int y = Window.getClientBoundsProperty().Height;

        getGraphicsDeviceProperty().setViewportProperty(Viewport(0, 0, x, y));
    }
}
