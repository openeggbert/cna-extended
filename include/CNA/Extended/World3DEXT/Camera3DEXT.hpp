// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// CNA::Extended::World3DEXT::Camera3DEXT -- new, non-upstream addition (there is no
// MonoGame.Extended C# source to port here; MonoGame.Extended is 2D-only). See 3d.md/
// plan3d.md at the repository root for the full design rationale and phase plan this file
// belongs to.
//
// Design note, recorded for anyone revisiting this file: an earlier draft of this design
// (3d.md's original wording) proposed deriving Camera3DEXT from this project's existing
// Camera<T>/IMovable/IRotatable. Reading those headers before writing this file showed
// that was wrong: Camera<T>'s interface only uses T for Position/Origin/Center -- half of
// it (getBoundingRectangleProperty() -> RectangleF, Contains(const Vector2&),
// Contains(const Rectangle&)) hardcodes 2D types regardless of T, because upstream
// MonoGame.Extended's Camera<T> was never actually instantiated with anything but
// Vector2. IMovable/IRotatable are flatly 2D too (Vector2 position, a single float
// rotation angle, no Quaternion). None of the three actually fit a 3D perspective camera,
// so Camera3DEXT is a standalone class instead -- see 3d.md section 6.1 for the full
// finding.
//
// Math adapted from cna-scene's own Camera3D (../cna-scene/include/CNA/Scene/Camera3D.hpp,
// read as reference material only -- cna-extended does not depend on cna-scene, see
// plan3d.md section 2): Matrix::CreateLookAt for the view matrix,
// Matrix::CreatePerspectiveFieldOfView for the projection matrix, both real, already-
// tested CNA math functions -- this file only stores the camera's own parameters and
// forwards them to that math, it introduces no new matrix math of its own.
#pragma once

#include "Microsoft/Xna/Framework/BoundingFrustum.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingFrustum;
    using Microsoft::Xna::Framework::Matrix;
    using Microsoft::Xna::Framework::Vector3;

    /**
     * @brief A perspective 3D camera: position/target/up plus the usual perspective
     * parameters (field of view, aspect ratio, near/far planes), producing CNA view and
     * projection matrices and a BoundingFrustum for visibility/culling tests.
     *
     * Not part of the MonoGame.Extended port -- MonoGame.Extended is 2D-only and has no
     * 3D camera. This is the entry point for CNA::Extended::World3DEXT, the new 3D scene
     * layer described in 3d.md/plan3d.md at the repository root.
     *
     * @code
     * #include <CNA/Extended/World3DEXT/Camera3DEXT.hpp>
     * #include <Microsoft/Xna/Framework/Vector3.hpp>
     *
     * using CNA::Extended::World3DEXT::Camera3DEXT;
     * using Microsoft::Xna::Framework::Vector3;
     *
     * void SetUpCamera(Camera3DEXT& camera)
     * {
     *     camera.setPositionProperty(Vector3(0.0f, 5.0f, 10.0f));
     *     camera.setTargetProperty(Vector3::Zero);
     *     camera.setAspectRatioProperty(1280.0f / 720.0f);
     *     // camera.GetViewMatrixEXT() / GetProjectionMatrixEXT() feed a BasicEffect's
     *     // View/Projection; GetBoundingFrustumEXT() drives visibility culling.
     * }
     * @endcode
     */
    class Camera3DEXT final
    {
    public:
        Camera3DEXT();

        /** @brief Gets the camera's position in world space. */
        [[nodiscard]] const Vector3& getPositionProperty() const { return position_; }
        /** @brief Sets the camera's position in world space. */
        void setPositionProperty(const Vector3& value) { position_ = value; }

        /** @brief Gets the world-space point the camera looks at. */
        [[nodiscard]] const Vector3& getTargetProperty() const { return target_; }
        /** @brief Sets the world-space point the camera looks at. */
        void setTargetProperty(const Vector3& value) { target_ = value; }

        /** @brief Gets the camera's up direction. */
        [[nodiscard]] const Vector3& getUpProperty() const { return up_; }
        /** @brief Sets the camera's up direction. */
        void setUpProperty(const Vector3& value) { up_ = value; }

        /** @brief Gets the vertical field of view, in radians. */
        [[nodiscard]] float getFieldOfViewProperty() const { return fieldOfView_; }
        /** @brief Sets the vertical field of view, in radians. */
        void setFieldOfViewProperty(float radians) { fieldOfView_ = radians; }

        /** @brief Gets the viewport aspect ratio (width / height). */
        [[nodiscard]] float getAspectRatioProperty() const { return aspectRatio_; }
        /** @brief Sets the viewport aspect ratio (width / height). */
        void setAspectRatioProperty(float value) { aspectRatio_ = value; }

        /** @brief Gets the distance to the near clipping plane. */
        [[nodiscard]] float getNearPlaneProperty() const { return nearPlane_; }
        /** @brief Sets the distance to the near clipping plane. */
        void setNearPlaneProperty(float value) { nearPlane_ = value; }

        /** @brief Gets the distance to the far clipping plane. */
        [[nodiscard]] float getFarPlaneProperty() const { return farPlane_; }
        /** @brief Sets the distance to the far clipping plane. */
        void setFarPlaneProperty(float value) { farPlane_ = value; }

        /** @brief Gets the view transformation matrix (Matrix::CreateLookAt over Position/Target/Up). */
        [[nodiscard]] Matrix GetViewMatrixEXT() const;

        /** @brief Gets the perspective projection matrix (Matrix::CreatePerspectiveFieldOfView). */
        [[nodiscard]] Matrix GetProjectionMatrixEXT() const;

        /** @brief Gets the BoundingFrustum for the camera's current view*projection, for visibility/culling tests. */
        [[nodiscard]] BoundingFrustum GetBoundingFrustumEXT() const;

    private:
        Vector3 position_;
        Vector3 target_;
        Vector3 up_;
        float fieldOfView_;
        float aspectRatio_;
        float nearPlane_;
        float farPlane_;
    };
}
