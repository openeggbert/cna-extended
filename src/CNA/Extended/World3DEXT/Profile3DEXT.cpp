// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/ConeProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Point()
    {
        return std::make_unique<PointProfile3DEXT>();
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Cone(const Vector3& direction, float halfAngle)
    {
        auto profile = std::make_unique<ConeProfile3DEXT>();
        profile->DirectionEXT = direction;
        profile->HalfAngleEXT = halfAngle;
        return profile;
    }
}
