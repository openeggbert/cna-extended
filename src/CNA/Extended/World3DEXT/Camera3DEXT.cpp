// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Camera3DEXT.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::MathHelper;

    Camera3DEXT::Camera3DEXT()
        : position_(0.0f, 2.0f, 5.0f),
          target_(Vector3::Zero),
          up_(Vector3::Up),
          fieldOfView_(MathHelper::PiOver4),
          aspectRatio_(16.0f / 9.0f),
          nearPlane_(0.1f),
          farPlane_(1000.0f)
    {
    }

    Matrix Camera3DEXT::GetViewMatrixEXT() const
    {
        return Matrix::CreateLookAt(position_, target_, up_);
    }

    Matrix Camera3DEXT::GetProjectionMatrixEXT() const
    {
        return Matrix::CreatePerspectiveFieldOfView(fieldOfView_, aspectRatio_, nearPlane_, farPlane_);
    }

    BoundingFrustum Camera3DEXT::GetBoundingFrustumEXT() const
    {
        return BoundingFrustum(GetViewMatrixEXT() * GetProjectionMatrixEXT());
    }
}
