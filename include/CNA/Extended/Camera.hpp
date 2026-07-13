// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Camera.cs. Fully abstract (all members pure virtual), so this
// is header-only -- no Camera.cpp. getBoundingRectangleProperty() returns RectangleF, which
// used to be forward-declared here (RectangleF wasn't ported yet); now #include'd directly since
// RectangleF landed in Phase 1 task 17 ("RectangleF family").
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::ContainmentType;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /**
     * @brief Represents an abstract camera.
     * @tparam T The position type for the camera. Typically Vector2 for 2D cameras or Vector3
     * for 3D cameras.
     */
    template <typename T>
    class Camera
    {
    public:
        virtual ~Camera() = default;

        /** @brief Gets the position of the camera in world coordinates. */
        [[nodiscard]] virtual T getPositionProperty() const = 0;
        /** @brief Sets the position of the camera in world coordinates. */
        virtual void setPositionProperty(const T& value) = 0;

        /** @brief Gets the rotation of the camera in radians. */
        [[nodiscard]] virtual float getRotationProperty() const = 0;
        /** @brief Sets the rotation of the camera in radians. */
        virtual void setRotationProperty(float value) = 0;

        /**
         * @brief Gets the zoom level of the camera. 1.0 is the default zoom level; values
         * greater than 1.0 zoom in, values less than 1.0 zoom out.
         */
        [[nodiscard]] virtual float getZoomProperty() const = 0;
        /** @brief Sets the zoom level of the camera. */
        virtual void setZoomProperty(float value) = 0;

        /** @brief Gets the minimum allowed zoom level. */
        [[nodiscard]] virtual float getMinimumZoomProperty() const = 0;
        /** @brief Sets the minimum allowed zoom level. */
        virtual void setMinimumZoomProperty(float value) = 0;

        /** @brief Gets the maximum allowed zoom level. */
        [[nodiscard]] virtual float getMaximumZoomProperty() const = 0;
        /** @brief Sets the maximum allowed zoom level. */
        virtual void setMaximumZoomProperty(float value) = 0;

        /** @brief Gets the axis-aligned bounding rectangle of the camera's view in world coordinates. */
        [[nodiscard]] virtual RectangleF getBoundingRectangleProperty() const = 0;

        /**
         * @brief Gets the origin point for rotation and zoom transformations. Typically set to
         * the center of the viewport.
         */
        [[nodiscard]] virtual T getOriginProperty() const = 0;
        /** @brief Sets the origin point for rotation and zoom transformations. */
        virtual void setOriginProperty(const T& value) = 0;

        /** @brief Gets the center position of the camera's view in world coordinates. */
        [[nodiscard]] virtual T getCenterProperty() const = 0;

        /**
         * @brief Moves the camera by the specified direction vector.
         * @param direction The direction and distance to move the camera.
         */
        virtual void Move(const T& direction) = 0;

        /**
         * @brief Rotates the camera by the specified amount.
         * @param deltaRadians The rotation amount in radians to apply.
         */
        virtual void Rotate(float deltaRadians) = 0;

        /**
         * @brief Increases the camera's zoom level by the specified amount.
         * @param deltaZoom The amount to increase the zoom by.
         */
        virtual void ZoomIn(float deltaZoom) = 0;

        /**
         * @brief Decreases the camera's zoom level by the specified amount.
         * @param deltaZoom The amount to decrease the zoom by.
         */
        virtual void ZoomOut(float deltaZoom) = 0;

        /**
         * @brief Positions the camera to look at the specified position.
         * @param position The world position to center the camera on.
         */
        virtual void LookAt(const T& position) = 0;

        /**
         * @brief Converts a position from world coordinates to screen coordinates.
         * @param worldPosition The position in world coordinates.
         */
        [[nodiscard]] virtual T WorldToScreen(const T& worldPosition) const = 0;

        /**
         * @brief Converts a position from screen coordinates to world coordinates.
         * @param screenPosition The position in screen coordinates.
         */
        [[nodiscard]] virtual T ScreenToWorld(const T& screenPosition) const = 0;

        /** @brief Gets the view transformation matrix for the camera. */
        [[nodiscard]] virtual Matrix GetViewMatrix() const = 0;

        /** @brief Gets the inverse of the view transformation matrix for the camera. */
        [[nodiscard]] virtual Matrix GetInverseViewMatrix() const = 0;

        /** @brief Gets the bounding frustum for the camera's view volume. */
        [[nodiscard]] virtual BoundingFrustum GetBoundingFrustum() const = 0;

        /**
         * @brief Determines whether the camera's view contains the specified point.
         * @param vector2 The point to test, in world coordinates.
         */
        [[nodiscard]] virtual ContainmentType Contains(const Vector2& vector2) const = 0;

        /**
         * @brief Determines whether the camera's view contains the specified rectangle.
         * @param rectangle The rectangle to test, in world coordinates.
         */
        [[nodiscard]] virtual ContainmentType Contains(const Rectangle& rectangle) const = 0;
    };
}
