// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's OrthographicCamera.cs. See OrthographicCamera.hpp for the
// ownership/interface-implementation design notes.
#include "CNA/Extended/OrthographicCamera.hpp"

#include "CNA/Extended/ViewportAdapters/ScalingViewportAdapter.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    OrthographicCamera::OrthographicCamera(GraphicsDevice& graphicsDevice)
        : ownedViewportAdapter_(std::in_place, graphicsDevice),
          viewportAdapter_(*ownedViewportAdapter_)
    {
        Initialize();
    }

    OrthographicCamera::OrthographicCamera(ViewportAdapters::ViewportAdapter& viewportAdapter)
        : viewportAdapter_(viewportAdapter)
    {
        Initialize();
    }

    void OrthographicCamera::Initialize()
    {
        rotation_ = 0.0f;
        setZoomProperty(1.0f);
        origin_ = Vector2(static_cast<float>(viewportAdapter_.getVirtualWidthProperty()) / 2.0f,
                           static_cast<float>(viewportAdapter_.getVirtualHeightProperty()) / 2.0f);
        setPositionProperty(Vector2::Zero);
    }

    void OrthographicCamera::setPositionProperty(const Vector2& value)
    {
        position_ = value;

        if (isClampedToWorldBounds_)
        {
            ClampPositionToWorldBounds();
        }
    }

    void OrthographicCamera::setZoomProperty(float value)
    {
        zoom_ = value;

        const bool canClampToWorldBounds = CanClampToWorldBounds();

        if (clampZoomToWorldBounds_ && canClampToWorldBounds)
        {
            ClampZoomToWorldBounds();
        }

        zoom_ = MathHelper::Clamp(zoom_, minimumZoom_, maximumZoom_);

        if (canClampToWorldBounds)
        {
            ClampPositionToWorldBounds();
        }
    }

    void OrthographicCamera::setMinimumZoomProperty(float value)
    {
        System::ArgumentOutOfRangeException::ThrowIfLessThan(value, 0.0f, "value");
        minimumZoom_ = value;

        const bool canClampToWorldBounds = CanClampToWorldBounds();

        if (clampZoomToWorldBounds_ && canClampToWorldBounds)
        {
            ClampZoomToWorldBounds();
        }

        zoom_ = MathHelper::Clamp(zoom_, minimumZoom_, maximumZoom_);

        if (canClampToWorldBounds)
        {
            ClampPositionToWorldBounds();
        }
    }

    void OrthographicCamera::setMaximumZoomProperty(float value)
    {
        System::ArgumentOutOfRangeException::ThrowIfLessThan(value, 0.0f, "value");
        maximumZoom_ = value;

        const bool canClampToWorldBounds = CanClampToWorldBounds();

        if (clampZoomToWorldBounds_ && canClampToWorldBounds)
        {
            ClampZoomToWorldBounds();
        }

        zoom_ = MathHelper::Clamp(zoom_, minimumZoom_, maximumZoom_);

        if (canClampToWorldBounds)
        {
            ClampPositionToWorldBounds();
        }
    }

    RectangleF OrthographicCamera::getBoundingRectangleProperty() const
    {
        const BoundingFrustum frustum = GetBoundingFrustum();
        const std::vector<Vector3> corners = frustum.GetCorners();
        const Vector3& topLeft = corners[0];
        const Vector3& bottomRight = corners[2];
        const float width = bottomRight.X - topLeft.X;
        const float height = bottomRight.Y - topLeft.Y;
        return RectangleF(topLeft.X, topLeft.Y, width, height);
    }

    void OrthographicCamera::setIsZoomClampedToWorldBoundsProperty(bool value)
    {
        clampZoomToWorldBounds_ = value;

        if (value)
        {
            ClampZoomToWorldBounds();
            zoom_ = MathHelper::Clamp(zoom_, minimumZoom_, maximumZoom_);
            ClampPositionToWorldBounds();
        }
    }

    void OrthographicCamera::Move(const Vector2& direction)
    {
        setPositionProperty(position_ + Vector2::Transform(direction, Matrix::CreateRotationZ(-rotation_)));
    }

    void OrthographicCamera::ZoomIn(float deltaZoom, const Vector2& zoomCenter)
    {
        const float previousZoom = zoom_;
        setZoomProperty(zoom_ + deltaZoom);

        if (zoom_ != previousZoom)
        {
            setPositionProperty(position_ + (zoomCenter - origin_ - position_) * ((zoom_ - previousZoom) / zoom_));
        }
    }

    void OrthographicCamera::ZoomOut(float deltaZoom, const Vector2& zoomCenter)
    {
        const float previousZoom = zoom_;
        setZoomProperty(zoom_ - deltaZoom);

        if (zoom_ != previousZoom)
        {
            setPositionProperty(position_ + (zoomCenter - origin_ - position_) * ((zoom_ - previousZoom) / zoom_));
        }
    }

    void OrthographicCamera::LookAt(const Vector2& position)
    {
        setPositionProperty(position - Vector2(static_cast<float>(viewportAdapter_.getVirtualWidthProperty()) / 2.0f,
                                                 static_cast<float>(viewportAdapter_.getVirtualHeightProperty()) / 2.0f));
    }

    Vector2 OrthographicCamera::WorldToScreen(const Vector2& worldPosition) const
    {
        Vector2 screenPosition = Vector2::Transform(worldPosition, GetViewMatrix());

        // For scaling viewport adapters, the viewport offset is part of the coordinate transformation.
        if (dynamic_cast<const ViewportAdapters::ScalingViewportAdapter*>(&viewportAdapter_) != nullptr)
        {
            const Viewport viewport = viewportAdapter_.getViewportProperty();
            screenPosition = screenPosition + Vector2(static_cast<float>(viewport.getXProperty()), static_cast<float>(viewport.getYProperty()));
        }

        return screenPosition;
    }

    Vector2 OrthographicCamera::ScreenToWorld(const Vector2& screenPosition) const
    {
        Vector2 adjustedScreenPosition = screenPosition;

        // For scaling viewport adapters, the viewport offset is part of the coordinate transformation.
        if (dynamic_cast<const ViewportAdapters::ScalingViewportAdapter*>(&viewportAdapter_) != nullptr)
        {
            const Viewport viewport = viewportAdapter_.getViewportProperty();
            adjustedScreenPosition = adjustedScreenPosition - Vector2(static_cast<float>(viewport.getXProperty()), static_cast<float>(viewport.getYProperty()));
        }

        return Vector2::Transform(adjustedScreenPosition, Matrix::Invert(GetViewMatrix()));
    }

    Matrix OrthographicCamera::GetViewMatrix(const Vector2& parallaxFactor) const
    {
        return GetVirtualViewMatrix(parallaxFactor) * viewportAdapter_.GetScaleMatrix();
    }

    Matrix OrthographicCamera::GetVirtualViewMatrix(const Vector2& parallaxFactor) const
    {
        return
            Matrix::CreateTranslation(Vector3(-position_ * parallaxFactor, 0.0f)) *
            Matrix::CreateTranslation(Vector3(-origin_, 0.0f)) *
            Matrix::CreateRotationZ(rotation_) *
            Matrix::CreateScale(zoom_, zoom_, 1.0f) *
            Matrix::CreateTranslation(Vector3(origin_, 0.0f));
    }

    Matrix OrthographicCamera::GetProjectionMatrix(const Matrix& viewMatrix) const
    {
        Matrix projection = Matrix::CreateOrthographicOffCenter(
            0.0f, static_cast<float>(viewportAdapter_.getVirtualWidthProperty()),
            static_cast<float>(viewportAdapter_.getVirtualHeightProperty()), 0.0f,
            -1.0f, 0.0f);
        Matrix result;
        Matrix::Multiply(viewMatrix, projection, result);
        return result;
    }

    BoundingFrustum OrthographicCamera::GetBoundingFrustum() const
    {
        const Matrix viewMatrix = GetVirtualViewMatrix();
        const Matrix projectionMatrix = GetProjectionMatrix(viewMatrix);
        return BoundingFrustum(projectionMatrix);
    }

    ContainmentType OrthographicCamera::Contains(const Vector2& vector2) const
    {
        return GetBoundingFrustum().Contains(Vector3(vector2.X, vector2.Y, 0.0f));
    }

    ContainmentType OrthographicCamera::Contains(const Rectangle& rectangle) const
    {
        const Vector3 max(static_cast<float>(rectangle.X + rectangle.Width), static_cast<float>(rectangle.Y + rectangle.Height), 0.5f);
        const Vector3 min(static_cast<float>(rectangle.X), static_cast<float>(rectangle.Y), 0.5f);
        const BoundingBox boundingBox(min, max);
        return GetBoundingFrustum().Contains(boundingBox);
    }

    void OrthographicCamera::EnableWorldBounds(const Rectangle& worldBounds)
    {
        worldBounds_ = worldBounds;
        isClampedToWorldBounds_ = true;
        ClampPositionToWorldBounds();
    }

    void OrthographicCamera::DisableWorldBounds()
    {
        worldBounds_ = Rectangle::Empty;
        isClampedToWorldBounds_ = false;
    }

    void OrthographicCamera::ClampZoomToWorldBounds()
    {
        // Calculate the size of the area the camera can see.
        const Vector2 cameraSize = Vector2(static_cast<float>(viewportAdapter_.getVirtualWidthProperty()),
                                            static_cast<float>(viewportAdapter_.getVirtualHeightProperty())) / zoom_;

        // Only enforce minimum zoom if the camera view is larger than world bounds.
        if (cameraSize.X > static_cast<float>(worldBounds_.Width) || cameraSize.Y > static_cast<float>(worldBounds_.Height))
        {
            const float minZoomX = static_cast<float>(viewportAdapter_.getVirtualWidthProperty()) / static_cast<float>(worldBounds_.Width);
            const float minZoomY = static_cast<float>(viewportAdapter_.getVirtualHeightProperty()) / static_cast<float>(worldBounds_.Height);
            const float minZoom = MathHelper::Max(minZoomX, minZoomY);

            if (zoom_ < minZoom)
            {
                zoom_ = minZoom;
            }
        }
    }

    void OrthographicCamera::ClampPositionToWorldBounds()
    {
        // Calculate the size of the area the camera can see.
        const Vector2 cameraSize = Vector2(static_cast<float>(viewportAdapter_.getVirtualWidthProperty()),
                                            static_cast<float>(viewportAdapter_.getVirtualHeightProperty())) / zoom_;

        // If the world bounds are smaller than the camera view, then we center the camera in the world bounds.
        if (static_cast<float>(worldBounds_.Width) < cameraSize.X || static_cast<float>(worldBounds_.Height) < cameraSize.Y)
        {
            const Point center = worldBounds_.getCenterProperty();
            position_ = Vector2(static_cast<float>(center.X), static_cast<float>(center.Y)) - origin_;
            return;
        }

        // Get the camera's top-left corner in world space.
        const Matrix inverseViewMatrix = GetInverseViewMatrix();
        const Vector2 cameraWorldMin = Vector2::Transform(Vector2::Zero, inverseViewMatrix);

        const Vector2 worldBoundsMin(static_cast<float>(worldBounds_.getLeftProperty()), static_cast<float>(worldBounds_.getTopProperty()));
        const Vector2 worldBoundsMax(static_cast<float>(worldBounds_.getRightProperty()), static_cast<float>(worldBounds_.getBottomProperty()));

        // Calculate difference between position and world-space top-left.
        const Vector2 positionOffset = position_ - cameraWorldMin;

        // Clamp the camera's world-space top-left corner, then apply the offset.
        position_ = Vector2::Clamp(cameraWorldMin, worldBoundsMin, worldBoundsMax - cameraSize) + positionOffset;
    }

    bool OrthographicCamera::CanClampToWorldBounds() const
    {
        if (!isClampedToWorldBounds_ || worldBounds_.Width <= 0 || worldBounds_.Height <= 0)
        {
            return false;
        }

        if (MathHelper::Distance(rotation_, 0.0f) >= 0.001f)
        {
            return false;
        }

        return true;
    }
}
