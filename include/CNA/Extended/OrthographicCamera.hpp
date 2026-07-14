// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's OrthographicCamera.cs: a sealed Camera<Vector2> that also
// implements IMovable/IRotatable. Deferred since Phase 1 (see plan.md's Phase 1 notes) pending
// `ViewportAdapters`, which landed in Phase 3 -- ported now as a necessary prerequisite for
// Phase 7's Tilemaps/Rendering/* module, all four of whose renderer classes require this type.
//
// Ownership: upstream's `_viewportAdapter` field is a plain GC-rooted reference -- when
// constructed via `OrthographicCamera(GraphicsDevice)`, C# implicitly keeps the new
// `DefaultViewportAdapter(graphicsDevice)` alive for as long as the camera references it, with no
// other owner needed. `ViewportAdapter` (this project's port) takes its `GraphicsDevice&` non-owning,
// by reference, matching the established "adapter never owns, always references an
// externally-owned, longer-lived object" convention -- so `OrthographicCamera` mirrors that same
// convention for its own `_viewportAdapter` field (`viewportAdapter_`, a `ViewportAdapter&`).
// That leaves the `GraphicsDevice&`-only constructor with nothing to hold the
// `DefaultViewportAdapter` it must create: `ownedViewportAdapter_` (new, no direct upstream field)
// is the owner in that case only, mirroring `Tilemap::ownedTextures_`/`BitmapFont::pageTextures_`'s
// identical C#-GC-vs-C++-value-return problem and solution. `viewportAdapter_` then references
// either the caller-supplied external adapter, or `*ownedViewportAdapter_` -- declaration order
// (below) matters here: `ownedViewportAdapter_` must be declared before `viewportAdapter_` so it's
// initialized first regardless of member-initializer-list order.
//
// `sealed` -> `final`. Implements `Camera<Vector2>`, `IMovable`, `IRotatable` simultaneously --
// `IMovable`/`IRotatable`'s `getPositionProperty`/`setPositionProperty`/`getRotationProperty`/
// `setRotationProperty` have identical signatures to `Camera<Vector2>`'s own (not a coincidence:
// upstream's `Camera<T>` doesn't itself implement these interfaces, but `OrthographicCamera`
// implements all three side by side) -- since the three base classes are unrelated (no shared
// ancestor, so no diamond), a single override in this class satisfies all matching pure virtuals
// from all three bases at once; standard, well-defined C++ behavior.
//
// Copy/move: deleted. `viewportAdapter_` is a reference member (can't be reseated on
// assignment), and this is a top-level, once-constructed-and-kept game object in upstream too
// (`sealed`, no upstream code ever copies or reassigns one) -- matching this project's
// `SpriteBatch`-style "construct once, reference thereafter" convention rather than adding
// move support upstream never needed.
#pragma once

#include "CNA/Extended/Camera.hpp"
#include "CNA/Extended/IMovable.hpp"
#include "CNA/Extended/IRotatable.hpp"
#include "CNA/Extended/ViewportAdapters/DefaultViewportAdapter.hpp"
#include "CNA/Extended/ViewportAdapters/ViewportAdapter.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"

#include <limits>
#include <optional>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Point;

    /** @brief An orthographic (2D) camera providing view and projection transformations for rendering within a 2D world. */
    class OrthographicCamera final : public Camera<Vector2>, public IMovable, public IRotatable
    {
    public:
        /** @brief Initializes a new OrthographicCamera using a new, self-owned DefaultViewportAdapter. */
        explicit OrthographicCamera(GraphicsDevice& graphicsDevice);

        /** @brief Initializes a new OrthographicCamera using the specified viewport adapter, which must outlive this camera. */
        explicit OrthographicCamera(ViewportAdapters::ViewportAdapter& viewportAdapter);

        ~OrthographicCamera() override = default;
        OrthographicCamera(const OrthographicCamera&) = delete;
        OrthographicCamera& operator=(const OrthographicCamera&) = delete;
        OrthographicCamera(OrthographicCamera&&) = delete;
        OrthographicCamera& operator=(OrthographicCamera&&) = delete;

        /** @brief Gets the position of the camera in world coordinates. */
        [[nodiscard]] Vector2 getPositionProperty() const override { return position_; }
        /**
         * @brief Sets the position of the camera in world coordinates. When world bounds are
         * enabled, the position is clamped so the view remains within them.
         */
        void setPositionProperty(const Vector2& value) override;

        [[nodiscard]] float getRotationProperty() const override { return rotation_; }
        void setRotationProperty(float value) override { rotation_ = value; }

        /**
         * @brief Sets the zoom level. When world bounds and zoom-clamping are enabled, the zoom
         * (and, transitively, the position) is clamped so the view remains within them.
         */
        [[nodiscard]] float getZoomProperty() const override { return zoom_; }
        void setZoomProperty(float value) override;

        [[nodiscard]] float getMinimumZoomProperty() const override { return minimumZoom_; }
        void setMinimumZoomProperty(float value) override;

        [[nodiscard]] float getMaximumZoomProperty() const override { return maximumZoom_; }
        void setMaximumZoomProperty(float value) override;

        [[nodiscard]] RectangleF getBoundingRectangleProperty() const override;

        [[nodiscard]] Vector2 getOriginProperty() const override { return origin_; }
        void setOriginProperty(const Vector2& value) override { origin_ = value; }

        [[nodiscard]] Vector2 getCenterProperty() const override { return position_ + origin_; }

        /** @brief Gets the bounding rectangle that defines the limits of the camera's movement. */
        [[nodiscard]] Rectangle getWorldBoundsProperty() const { return worldBounds_; }

        /** @brief Gets whether the camera is currently constrained within world bounds. */
        [[nodiscard]] bool getIsClampedToWorldBoundsProperty() const { return isClampedToWorldBounds_; }

        /**
         * @brief Gets/sets whether zoom should be clamped to world bounds. Only has effect when
         * IsClampedToWorldBounds is true.
         */
        [[nodiscard]] bool getIsZoomClampedToWorldBoundsProperty() const { return clampZoomToWorldBounds_; }
        void setIsZoomClampedToWorldBoundsProperty(bool value);

        void Move(const Vector2& direction) override;
        void Rotate(float deltaRadians) override { rotation_ += deltaRadians; }
        void ZoomIn(float deltaZoom) override { setZoomProperty(zoom_ + deltaZoom); }
        void ZoomOut(float deltaZoom) override { setZoomProperty(zoom_ - deltaZoom); }

        /** @brief Increases zoom by @p deltaZoom, adjusting position so @p zoomCenter stays fixed on screen. */
        void ZoomIn(float deltaZoom, const Vector2& zoomCenter);
        /** @brief Decreases zoom by @p deltaZoom, adjusting position so @p zoomCenter stays fixed on screen. */
        void ZoomOut(float deltaZoom, const Vector2& zoomCenter);

        /** @brief Positions the camera so @p position appears at the center of the viewport. */
        void LookAt(const Vector2& position) override;

        [[nodiscard]] Vector2 WorldToScreen(float x, float y) const { return WorldToScreen(Vector2(x, y)); }
        [[nodiscard]] Vector2 WorldToScreen(const Vector2& worldPosition) const override;

        [[nodiscard]] Vector2 ScreenToWorld(float x, float y) const { return ScreenToWorld(Vector2(x, y)); }
        [[nodiscard]] Vector2 ScreenToWorld(const Vector2& screenPosition) const override;

        /** @brief Gets the view matrix, applying the specified parallax factor to the camera position. (1,1) applies none. */
        [[nodiscard]] Matrix GetViewMatrix(const Vector2& parallaxFactor) const;

        [[nodiscard]] Matrix GetViewMatrix() const override { return GetViewMatrix(Vector2::One); }
        [[nodiscard]] Matrix GetInverseViewMatrix() const override { return Matrix::Invert(GetViewMatrix()); }

        [[nodiscard]] BoundingFrustum GetBoundingFrustum() const override;

        /** @brief Determines whether the camera's view contains the specified point. */
        [[nodiscard]] ContainmentType Contains(const Point& point) const
        {
            return Contains(Vector2(static_cast<float>(point.X), static_cast<float>(point.Y)));
        }
        [[nodiscard]] ContainmentType Contains(const Vector2& vector2) const override;
        [[nodiscard]] ContainmentType Contains(const Rectangle& rectangle) const override;

        /** @brief Enables the world-bounds constraint and sets the bounding rectangle limiting camera movement/zoom. */
        void EnableWorldBounds(const Rectangle& worldBounds);
        /** @brief Disables the world-bounds constraint, resetting WorldBounds to Rectangle::Empty. */
        void DisableWorldBounds();

    private:
        void Initialize();
        [[nodiscard]] Matrix GetVirtualViewMatrix(const Vector2& parallaxFactor) const;
        [[nodiscard]] Matrix GetVirtualViewMatrix() const { return GetVirtualViewMatrix(Vector2::One); }
        [[nodiscard]] Matrix GetProjectionMatrix(const Matrix& viewMatrix) const;
        void ClampZoomToWorldBounds();
        void ClampPositionToWorldBounds();
        [[nodiscard]] bool CanClampToWorldBounds() const;

        std::optional<ViewportAdapters::DefaultViewportAdapter> ownedViewportAdapter_;
        ViewportAdapters::ViewportAdapter& viewportAdapter_;
        float maximumZoom_ = std::numeric_limits<float>::max();
        float minimumZoom_ = 0.0f;
        float zoom_ = 0.0f;
        Vector2 position_;
        float rotation_ = 0.0f;
        Vector2 origin_;
        Rectangle worldBounds_;
        bool clampZoomToWorldBounds_ = false;
        bool isClampedToWorldBounds_ = false;
    };
}
