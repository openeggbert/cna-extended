// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for the abstract Camera<T> (nothing
// upstream instantiates Camera<T> directly either -- only OrthographicCamera, which is
// Camera<Vector2>, does). These tests use a minimal concrete Camera<Vector2> test double
// (mirroring InterfaceTests.cpp's pattern) to confirm the interface is implementable and that
// the getX/setXProperty() pattern round-trips, now that RectangleF (needed by
// getBoundingRectangleProperty()) has landed -- see Camera.hpp's header comment.
#include "CNA/Extended/Camera.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    namespace
    {
        class SimpleCamera final : public Camera<Vector2>
        {
        public:
            [[nodiscard]] Vector2 getPositionProperty() const override { return position_; }
            void setPositionProperty(const Vector2& value) override { position_ = value; }

            [[nodiscard]] float getRotationProperty() const override { return rotation_; }
            void setRotationProperty(const float value) override { rotation_ = value; }

            [[nodiscard]] float getZoomProperty() const override { return zoom_; }
            void setZoomProperty(const float value) override { zoom_ = value; }

            [[nodiscard]] float getMinimumZoomProperty() const override { return minimumZoom_; }
            void setMinimumZoomProperty(const float value) override { minimumZoom_ = value; }

            [[nodiscard]] float getMaximumZoomProperty() const override { return maximumZoom_; }
            void setMaximumZoomProperty(const float value) override { maximumZoom_ = value; }

            [[nodiscard]] RectangleF getBoundingRectangleProperty() const override
            {
                return RectangleF(position_.X - 100.0f, position_.Y - 100.0f, 200.0f, 200.0f);
            }

            [[nodiscard]] Vector2 getOriginProperty() const override { return origin_; }
            void setOriginProperty(const Vector2& value) override { origin_ = value; }

            [[nodiscard]] Vector2 getCenterProperty() const override { return position_; }

            void Move(const Vector2& direction) override { position_ = position_ + direction; }
            void Rotate(const float deltaRadians) override { rotation_ += deltaRadians; }
            void ZoomIn(const float deltaZoom) override { zoom_ += deltaZoom; }
            void ZoomOut(const float deltaZoom) override { zoom_ -= deltaZoom; }
            void LookAt(const Vector2& targetPosition) override { position_ = targetPosition; }

            [[nodiscard]] Vector2 WorldToScreen(const Vector2& worldPosition) const override { return worldPosition - position_; }
            [[nodiscard]] Vector2 ScreenToWorld(const Vector2& screenPosition) const override { return screenPosition + position_; }

            [[nodiscard]] Matrix GetViewMatrix() const override { return Matrix::CreateTranslation(-position_.X, -position_.Y, 0.0f); }
            [[nodiscard]] Matrix GetInverseViewMatrix() const override { return Matrix::CreateTranslation(position_.X, position_.Y, 0.0f); }

            [[nodiscard]] BoundingFrustum GetBoundingFrustum() const override { return BoundingFrustum(GetViewMatrix()); }

            [[nodiscard]] ContainmentType Contains(const Vector2& vector2) const override
            {
                return getBoundingRectangleProperty().Contains(vector2) ? ContainmentType::Contains : ContainmentType::Disjoint;
            }

            [[nodiscard]] ContainmentType Contains(const Rectangle& rectangle) const override
            {
                return Contains(Vector2(static_cast<float>(rectangle.X), static_cast<float>(rectangle.Y)));
            }

        private:
            Vector2 position_{};
            float rotation_ = 0.0f;
            float zoom_ = 1.0f;
            float minimumZoom_ = 0.1f;
            float maximumZoom_ = 10.0f;
            Vector2 origin_{};
        };
    }

    TEST(CameraTests, PositionRoundTrips)
    {
        SimpleCamera camera;
        camera.setPositionProperty(Vector2(10.0f, 20.0f));
        EXPECT_FLOAT_EQ(camera.getPositionProperty().X, 10.0f);
        EXPECT_FLOAT_EQ(camera.getPositionProperty().Y, 20.0f);
    }

    TEST(CameraTests, ZoomRoundTrips)
    {
        SimpleCamera camera;
        camera.setZoomProperty(2.0f);
        EXPECT_FLOAT_EQ(camera.getZoomProperty(), 2.0f);
    }

    TEST(CameraTests, MoveTranslatesPosition)
    {
        SimpleCamera camera;
        camera.setPositionProperty(Vector2(5.0f, 5.0f));
        camera.Move(Vector2(1.0f, -1.0f));
        EXPECT_FLOAT_EQ(camera.getPositionProperty().X, 6.0f);
        EXPECT_FLOAT_EQ(camera.getPositionProperty().Y, 4.0f);
    }

    TEST(CameraTests, ZoomInOutAdjustsZoom)
    {
        SimpleCamera camera;
        camera.setZoomProperty(1.0f);
        camera.ZoomIn(0.5f);
        EXPECT_FLOAT_EQ(camera.getZoomProperty(), 1.5f);
        camera.ZoomOut(0.25f);
        EXPECT_FLOAT_EQ(camera.getZoomProperty(), 1.25f);
    }

    TEST(CameraTests, LookAtSetsPosition)
    {
        SimpleCamera camera;
        camera.LookAt(Vector2(42.0f, -7.0f));
        EXPECT_FLOAT_EQ(camera.getPositionProperty().X, 42.0f);
        EXPECT_FLOAT_EQ(camera.getPositionProperty().Y, -7.0f);
    }

    TEST(CameraTests, BoundingRectangleReflectsPosition)
    {
        SimpleCamera camera;
        camera.setPositionProperty(Vector2(0.0f, 0.0f));
        const RectangleF bounds = camera.getBoundingRectangleProperty();
        EXPECT_FLOAT_EQ(bounds.X, -100.0f);
        EXPECT_FLOAT_EQ(bounds.Y, -100.0f);
        EXPECT_FLOAT_EQ(bounds.Width, 200.0f);
        EXPECT_FLOAT_EQ(bounds.Height, 200.0f);
    }

    TEST(CameraTests, ContainsPointInsideBoundingRectangle)
    {
        SimpleCamera camera;
        camera.setPositionProperty(Vector2(0.0f, 0.0f));
        EXPECT_EQ(camera.Contains(Vector2(0.0f, 0.0f)), ContainmentType::Contains);
        EXPECT_EQ(camera.Contains(Vector2(1000.0f, 1000.0f)), ContainmentType::Disjoint);
    }

    TEST(CameraTests, WorldToScreenAndBackRoundTrips)
    {
        SimpleCamera camera;
        camera.setPositionProperty(Vector2(3.0f, 4.0f));
        const Vector2 world(10.0f, 10.0f);
        const Vector2 screen = camera.WorldToScreen(world);
        const Vector2 backToWorld = camera.ScreenToWorld(screen);
        EXPECT_FLOAT_EQ(backToWorld.X, world.X);
        EXPECT_FLOAT_EQ(backToWorld.Y, world.Y);
    }
}
