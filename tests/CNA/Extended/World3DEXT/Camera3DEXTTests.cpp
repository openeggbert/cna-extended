// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Camera3DEXT (see 3d.md/plan3d.md) -- there is no upstream
// MonoGame.Extended test suite to port here, since MonoGame.Extended has no 3D camera.
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"

#include "Microsoft/Xna/Framework/ContainmentType.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::ContainmentType;
    using Microsoft::Xna::Framework::MathHelper;

    TEST(Camera3DEXTTests, DefaultConstructedHasSensibleDefaults)
    {
        const Camera3DEXT camera;

        EXPECT_EQ(camera.getTargetProperty(), Vector3::Zero);
        EXPECT_EQ(camera.getUpProperty(), Vector3::Up);
        EXPECT_FLOAT_EQ(camera.getFieldOfViewProperty(), MathHelper::PiOver4);
        EXPECT_GT(camera.getAspectRatioProperty(), 0.0f);
        EXPECT_GT(camera.getNearPlaneProperty(), 0.0f);
        EXPECT_GT(camera.getFarPlaneProperty(), camera.getNearPlaneProperty());
        // A default camera is positioned somewhere other than its own target, so it has a
        // real, non-degenerate look direction.
        EXPECT_NE(camera.getPositionProperty(), camera.getTargetProperty());
    }

    TEST(Camera3DEXTTests, PropertySettersRoundTrip)
    {
        Camera3DEXT camera;

        camera.setPositionProperty(Vector3(1.0f, 2.0f, 3.0f));
        camera.setTargetProperty(Vector3(4.0f, 5.0f, 6.0f));
        camera.setUpProperty(Vector3(0.0f, 0.0f, 1.0f));
        camera.setFieldOfViewProperty(1.0f);
        camera.setAspectRatioProperty(4.0f / 3.0f);
        camera.setNearPlaneProperty(0.5f);
        camera.setFarPlaneProperty(500.0f);

        EXPECT_EQ(camera.getPositionProperty(), Vector3(1.0f, 2.0f, 3.0f));
        EXPECT_EQ(camera.getTargetProperty(), Vector3(4.0f, 5.0f, 6.0f));
        EXPECT_EQ(camera.getUpProperty(), Vector3(0.0f, 0.0f, 1.0f));
        EXPECT_FLOAT_EQ(camera.getFieldOfViewProperty(), 1.0f);
        EXPECT_FLOAT_EQ(camera.getAspectRatioProperty(), 4.0f / 3.0f);
        EXPECT_FLOAT_EQ(camera.getNearPlaneProperty(), 0.5f);
        EXPECT_FLOAT_EQ(camera.getFarPlaneProperty(), 500.0f);
    }

    TEST(Camera3DEXTTests, GetViewMatrixEXT_MatchesCreateLookAt)
    {
        Camera3DEXT camera;
        camera.setPositionProperty(Vector3(0.0f, 5.0f, 10.0f));
        camera.setTargetProperty(Vector3::Zero);
        camera.setUpProperty(Vector3::Up);

        const Matrix actual = camera.GetViewMatrixEXT();
        const Matrix expected = Matrix::CreateLookAt(Vector3(0.0f, 5.0f, 10.0f), Vector3::Zero, Vector3::Up);

        EXPECT_EQ(actual, expected);
    }

    TEST(Camera3DEXTTests, GetProjectionMatrixEXT_MatchesCreatePerspectiveFieldOfView)
    {
        Camera3DEXT camera;
        camera.setFieldOfViewProperty(MathHelper::PiOver4);
        camera.setAspectRatioProperty(1.5f);
        camera.setNearPlaneProperty(1.0f);
        camera.setFarPlaneProperty(100.0f);

        const Matrix actual = camera.GetProjectionMatrixEXT();
        const Matrix expected = Matrix::CreatePerspectiveFieldOfView(MathHelper::PiOver4, 1.5f, 1.0f, 100.0f);

        EXPECT_EQ(actual, expected);
    }

    TEST(Camera3DEXTTests, GetBoundingFrustumEXT_ContainsPointAtTarget)
    {
        Camera3DEXT camera;
        camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
        camera.setTargetProperty(Vector3::Zero);
        camera.setNearPlaneProperty(0.1f);
        camera.setFarPlaneProperty(100.0f);

        const auto frustum = camera.GetBoundingFrustumEXT();

        EXPECT_EQ(frustum.Contains(Vector3::Zero), ContainmentType::Contains);
    }

    TEST(Camera3DEXTTests, GetBoundingFrustumEXT_DoesNotContainPointBehindCamera)
    {
        Camera3DEXT camera;
        camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
        camera.setTargetProperty(Vector3::Zero);
        camera.setNearPlaneProperty(0.1f);
        camera.setFarPlaneProperty(100.0f);

        const auto frustum = camera.GetBoundingFrustumEXT();
        const Vector3 behindCamera(0.0f, 0.0f, 50.0f); // further from origin than the camera itself, on the same side as the camera -> behind it

        EXPECT_EQ(frustum.Contains(behindCamera), ContainmentType::Disjoint);
    }

    TEST(Camera3DEXTTests, GetBoundingFrustumEXT_DoesNotContainPointBeyondFarPlane)
    {
        Camera3DEXT camera;
        camera.setPositionProperty(Vector3(0.0f, 0.0f, 10.0f));
        camera.setTargetProperty(Vector3::Zero);
        camera.setNearPlaneProperty(0.1f);
        camera.setFarPlaneProperty(20.0f);

        const auto frustum = camera.GetBoundingFrustumEXT();
        const Vector3 beyondFarPlane(0.0f, 0.0f, -1000.0f);

        EXPECT_EQ(frustum.Contains(beyondFarPlane), ContainmentType::Disjoint);
    }
}
