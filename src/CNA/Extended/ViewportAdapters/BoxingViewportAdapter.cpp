// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ViewportAdapters/BoxingViewportAdapter.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

namespace CNA::Extended::ViewportAdapters
{
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Rectangle;

    BoxingViewportAdapter::BoxingViewportAdapter(
        GameWindow& window, GraphicsDevice& graphicsDevice, int virtualWidth, int virtualHeight, int horizontalBleed, int verticalBleed)
        : ScalingViewportAdapter(graphicsDevice, virtualWidth, virtualHeight),
          window_(window),
          horizontalBleed_(horizontalBleed),
          verticalBleed_(verticalBleed)
    {
        clientSizeChangedToken_ = window_.ClientSizeChanged.Add(
            [this](System::Object* sender, const System::EventArgs& eventArgs) { OnClientSizeChanged(sender, eventArgs); });
    }

    void BoxingViewportAdapter::Dispose()
    {
        window_.ClientSizeChanged.Remove(clientSizeChangedToken_);
        ScalingViewportAdapter::Dispose();
    }

    void BoxingViewportAdapter::OnClientSizeChanged(System::Object*, const System::EventArgs&)
    {
        const Rectangle clientBounds = window_.getClientBoundsProperty();

        const float worldScaleX = static_cast<float>(clientBounds.Width) / static_cast<float>(getVirtualWidthProperty());
        const float worldScaleY = static_cast<float>(clientBounds.Height) / static_cast<float>(getVirtualHeightProperty());

        const float safeScaleX = static_cast<float>(clientBounds.Width) / static_cast<float>(getVirtualWidthProperty() - horizontalBleed_);
        const float safeScaleY = static_cast<float>(clientBounds.Height) / static_cast<float>(getVirtualHeightProperty() - verticalBleed_);

        const float worldScale = MathHelper::Max(worldScaleX, worldScaleY);
        const float safeScale = MathHelper::Min(safeScaleX, safeScaleY);
        const float scale = MathHelper::Min(worldScale, safeScale);

        const int width = static_cast<int>(scale * static_cast<float>(getVirtualWidthProperty()) + 0.5f);
        const int height = static_cast<int>(scale * static_cast<float>(getVirtualHeightProperty()) + 0.5f);

        if (height >= clientBounds.Height && width < clientBounds.Width)
        {
            boxingMode_ = BoxingMode::Pillarbox;
        }
        else
        {
            // *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY ***
            // Upstream compares `width >= clientBounds.Height` here (the WIDTH against the
            // client area's HEIGHT dimension), not `clientBounds.Width` as the Pillarbox branch
            // above does. This looks like a copy-paste error, but is reproduced exactly -- not
            // "fixed" to compare width against Width -- per this project's "port 1:1, no
            // simplification" requirement.
            if (width >= clientBounds.Height && height <= clientBounds.Height)
            {
                boxingMode_ = BoxingMode::Letterbox;
            }
            else
            {
                boxingMode_ = BoxingMode::None;
            }
        }

        const int x = clientBounds.Width / 2 - width / 2;
        const int y = clientBounds.Height / 2 - height / 2;
        getGraphicsDeviceProperty().setViewportProperty(Viewport(x, y, width, height));
    }

    void BoxingViewportAdapter::Reset()
    {
        ScalingViewportAdapter::Reset();
        OnClientSizeChanged(this, System::EventArgs::Empty);
    }

    Point BoxingViewportAdapter::PointToScreen(int x, int y) const
    {
        const Viewport viewport = getGraphicsDeviceProperty().getViewportProperty();
        return ViewportAdapter::PointToScreen(x - viewport.getXProperty(), y - viewport.getYProperty());
    }
}
