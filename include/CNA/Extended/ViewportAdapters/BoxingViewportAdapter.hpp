// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ViewportAdapters/BoxingViewportAdapter.cs: a ScalingViewportAdapter
// that letterboxes/pillarboxes to preserve aspect ratio while allowing a "safe" bleed area to be
// cropped, tracking live window resizes via GameWindow::ClientSizeChanged.
//
// C#'s `window.ClientSizeChanged -= OnClientSizeChanged;` (Dispose()) has no direct C++ equivalent
// -- sharp-runtime's System::EventHandler<T> has no operator-= (unsubscribe is token-based, via
// Remove(Token), not handler-reference-equality). The Token returned by the constructor's
// ClientSizeChanged += subscription is stored and passed to Remove() in Dispose(), achieving the
// same effect through sharp-runtime's own established unsubscription mechanism.
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- see OnClientSizeChanged's inline comment ***
// The Letterbox-vs-None branch compares `width >= clientBounds.Height` (WIDTH against the client
// area's HEIGHT), not `clientBounds.Width` as the Pillarbox branch above it does -- looks like a
// copy-paste error, reproduced exactly rather than "fixed."
#pragma once

#include "CNA/Extended/ViewportAdapters/ScalingViewportAdapter.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "System/EventArgs.hpp"
#include "System/EventHandler.hpp"
#include "System/Object.hpp"

#include <string>

namespace CNA::Extended::ViewportAdapters
{
    using Microsoft::Xna::Framework::GameWindow;

    /** @brief Identifies which axis (if either) BoxingViewportAdapter is currently letterboxing/pillarboxing on. */
    enum class BoxingMode
    {
        None,
        Letterbox,
        Pillarbox
    };

    /** @brief A ScalingViewportAdapter that letterboxes/pillarboxes to preserve aspect ratio, with a croppable safe-bleed margin. */
    class BoxingViewportAdapter : public ScalingViewportAdapter
    {
    public:
        /**
         * @brief Initializes a new BoxingViewportAdapter.
         * @param window The game window to track for live resizes. Must outlive this adapter.
         * @param graphicsDevice The graphics device this adapter is bound to. Must outlive this adapter.
         * @param virtualWidth The virtual (design-time) width, in pixels.
         * @param virtualHeight The virtual (design-time) height, in pixels.
         * @param horizontalBleed Size of horizontal bleed areas (from left/right edges) which can be safely cut off.
         * @param verticalBleed Size of vertical bleed areas (from top/bottom edges) which can be safely cut off.
         */
        BoxingViewportAdapter(GameWindow& window, GraphicsDevice& graphicsDevice, int virtualWidth, int virtualHeight, int horizontalBleed = 0,
            int verticalBleed = 0);

        void Dispose() override;

        [[nodiscard]] int getHorizontalBleedProperty() const { return horizontalBleed_; }
        [[nodiscard]] int getVerticalBleedProperty() const { return verticalBleed_; }
        [[nodiscard]] BoxingMode getBoxingModeProperty() const { return boxingMode_; }

        void Reset() override;

        [[nodiscard]] Point PointToScreen(int x, int y) const override;

        [[nodiscard]] const std::string& GetTypeName() const override
        {
            static const std::string typeName = "MonoGame.Extended.ViewportAdapters.BoxingViewportAdapter";
            return typeName;
        }

    private:
        void OnClientSizeChanged(System::Object* sender, const System::EventArgs& eventArgs);

        GameWindow& window_;
        System::EventHandler<System::EventArgs>::Token clientSizeChangedToken_;
        int horizontalBleed_;
        int verticalBleed_;
        BoxingMode boxingMode_ = BoxingMode::None;
    };
}
