// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Profile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/BoxFillProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/BoxProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/BoxUniformProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/CircleProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/ConeProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/LineProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/PointProfile3DEXT.hpp"
#include "CNA/Extended/World3DEXT/RingProfile3DEXT.hpp"

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

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Line(const Vector3& axis, float length)
    {
        auto profile = std::make_unique<LineProfile3DEXT>();
        profile->AxisEXT = axis;
        profile->LengthEXT = length;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Line(const Vector3& axis, float length, LineRadiation3DEXT radiate)
    {
        auto profile = std::make_unique<LineProfile3DEXT>();
        profile->AxisEXT = axis;
        profile->LengthEXT = length;
        profile->RadiateEXT = radiate;
        profile->DirectionEXT = Vector3::Zero;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Line(const Vector3& axis, float length, LineRadiation3DEXT radiate, const Vector3& direction)
    {
        auto profile = std::make_unique<LineProfile3DEXT>();
        profile->AxisEXT = axis;
        profile->LengthEXT = length;
        profile->RadiateEXT = radiate;
        profile->DirectionEXT = direction;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Ring(float radius, CircleRadiation3DEXT radiate)
    {
        auto profile = std::make_unique<RingProfile3DEXT>();
        profile->RadiusEXT = radius;
        profile->RadiateEXT = radiate;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Box(float width, float height, float depth)
    {
        auto profile = std::make_unique<BoxProfile3DEXT>();
        profile->WidthEXT = width;
        profile->HeightEXT = height;
        profile->DepthEXT = depth;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::BoxFill(float width, float height, float depth)
    {
        auto profile = std::make_unique<BoxFillProfile3DEXT>();
        profile->WidthEXT = width;
        profile->HeightEXT = height;
        profile->DepthEXT = depth;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::BoxUniform(float width, float height, float depth)
    {
        auto profile = std::make_unique<BoxUniformProfile3DEXT>();
        profile->WidthEXT = width;
        profile->HeightEXT = height;
        profile->DepthEXT = depth;
        return profile;
    }

    std::unique_ptr<Profile3DEXT> Profile3DEXT::Circle(float radius, CircleRadiation3DEXT radiate)
    {
        auto profile = std::make_unique<CircleProfile3DEXT>();
        profile->RadiusEXT = radius;
        profile->RadiateEXT = radiate;
        return profile;
    }
}
