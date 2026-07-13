// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's ViewportAdapters/ViewportAdapter.cs: an abstract base decoupling
// a game's virtual (design-time) resolution from the actual window/backbuffer resolution. Upstream
// lives in the MonoGame.Extended.ViewportAdapters namespace; ported into a matching
// CNA::Extended::ViewportAdapters sub-namespace, per this project's established
// sub-namespace-per-module convention.
//
// `IDisposable` -> sharp-runtime's System::IDisposable. GraphicsDevice is taken by reference (not
// pointer/value), matching CNA's own SpriteBatch::SpriteBatch(GraphicsDevice&) convention rather
// than inventing a new one -- the adapter never owns the device, only references an
// externally-owned, longer-lived one, same as upstream's GC-backed reference field.
#pragma once

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Viewport.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "System/IDisposable.hpp"
#include "System/Object.hpp"

namespace CNA::Extended::ViewportAdapters
{
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    /**
     * @brief Decouples a game's virtual (design-time) resolution from the actual window/backbuffer resolution.
     * @remark Derives from System::Object (in addition to upstream's IDisposable) purely so
     * `this` converts to System::Object* for System::EventHandler<T>::Raise()'s sender parameter
     * -- needed by BoxingViewportAdapter's Reset(), which raises its own ClientSizeChanged
     * handler with `this` as the sender, matching the established GameTimer.hpp/FramesPerSecondCounter.hpp
     * pattern elsewhere in this project for any type that both implements a CNA interface and
     * needs to appear as an EventHandler<T> sender.
     */
    class ViewportAdapter : public System::Object, public System::IDisposable
    {
    public:
        ~ViewportAdapter() override = default;

        /** @brief Releases resources held by this adapter. The base implementation does nothing. */
        void Dispose() override {}

        /** @brief Gets the graphics device this adapter is bound to. */
        [[nodiscard]] GraphicsDevice& getGraphicsDeviceProperty() const { return graphicsDevice_; }

        /** @brief Gets the graphics device's current viewport. */
        [[nodiscard]] Viewport getViewportProperty() const { return graphicsDevice_.getViewportProperty(); }

        /** @brief Gets the virtual (design-time) width, in pixels. */
        [[nodiscard]] virtual int getVirtualWidthProperty() const = 0;

        /** @brief Gets the virtual (design-time) height, in pixels. */
        [[nodiscard]] virtual int getVirtualHeightProperty() const = 0;

        /** @brief Gets the actual viewport width, in pixels. */
        [[nodiscard]] virtual int getViewportWidthProperty() const = 0;

        /** @brief Gets the actual viewport height, in pixels. */
        [[nodiscard]] virtual int getViewportHeightProperty() const = 0;

        /** @brief Gets a rectangle spanning (0, 0) to (VirtualWidth, VirtualHeight). */
        [[nodiscard]] Rectangle getBoundingRectangleProperty() const { return Rectangle(0, 0, getVirtualWidthProperty(), getVirtualHeightProperty()); }

        /** @brief Gets the center point of BoundingRectangle. */
        [[nodiscard]] Point getCenterProperty() const { return getBoundingRectangleProperty().getCenterProperty(); }

        /** @brief Gets the matrix that scales from virtual coordinates to actual viewport coordinates. */
        [[nodiscard]] virtual Matrix GetScaleMatrix() const = 0;

        /** @brief Converts a virtual-space point to screen (actual viewport) space. */
        [[nodiscard]] Point PointToScreen(const Point& point) const { return PointToScreen(point.X, point.Y); }

        /** @brief Converts a virtual-space point to screen (actual viewport) space. */
        [[nodiscard]] virtual Point PointToScreen(int x, int y) const;

        /** @brief Recomputes any cached viewport/scale state. The base implementation does nothing. */
        virtual void Reset() {}

    protected:
        /**
         * @brief Initializes a new ViewportAdapter bound to the specified graphics device.
         * @param graphicsDevice The graphics device this adapter is bound to. Must outlive this adapter.
         */
        explicit ViewportAdapter(GraphicsDevice& graphicsDevice) : graphicsDevice_(graphicsDevice) {}

    private:
        GraphicsDevice& graphicsDevice_;
    };
}
