// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/Profile.hpp"

#include "CNA/Extended/Particles/Profiles/BoxFillProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxProfile.hpp"
#include "CNA/Extended/Particles/Profiles/BoxUniformProfile.hpp"
#include "CNA/Extended/Particles/Profiles/CircleProfile.hpp"
#include "CNA/Extended/Particles/Profiles/LineProfile.hpp"
#include "CNA/Extended/Particles/Profiles/PointProfile.hpp"
#include "CNA/Extended/Particles/Profiles/RingProfile.hpp"
#include "CNA/Extended/Particles/Profiles/SprayProfile.hpp"

namespace CNA::Extended::Particles::Profiles
{
    std::unique_ptr<Profile> Profile::Point()
    {
        return std::make_unique<PointProfile>();
    }

    std::unique_ptr<Profile> Profile::Line(const Vector2& axis, float length)
    {
        auto profile = std::make_unique<LineProfile>();
        profile->Axis = axis;
        profile->Length = length;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Line(const Vector2& axis, float length, LineRadiation radiate)
    {
        auto profile = std::make_unique<LineProfile>();
        profile->Axis = axis;
        profile->Length = length;
        profile->Radiate = radiate;
        profile->Direction = Vector2::Zero;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Line(const Vector2& axis, float length, LineRadiation radiate, const Vector2& direction)
    {
        auto profile = std::make_unique<LineProfile>();
        profile->Axis = axis;
        profile->Length = length;
        profile->Radiate = radiate;
        profile->Direction = direction;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Ring(float radius, CircleRadiation radiate)
    {
        auto profile = std::make_unique<RingProfile>();
        profile->Radius = radius;
        profile->Radiate = radiate;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Box(float width, float height)
    {
        auto profile = std::make_unique<BoxProfile>();
        profile->Width = width;
        profile->Height = height;
        return profile;
    }

    std::unique_ptr<Profile> Profile::BoxFill(float width, float height)
    {
        auto profile = std::make_unique<BoxFillProfile>();
        profile->Width = width;
        profile->Height = height;
        return profile;
    }

    std::unique_ptr<Profile> Profile::BoxUniform(float width, float height)
    {
        auto profile = std::make_unique<BoxUniformProfile>();
        profile->Width = width;
        profile->Height = height;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Circle(float radius, CircleRadiation radiate)
    {
        auto profile = std::make_unique<CircleProfile>();
        profile->Radius = radius;
        profile->Radiate = radiate;
        return profile;
    }

    std::unique_ptr<Profile> Profile::Spray(const Vector2& direction, float spread)
    {
        auto profile = std::make_unique<SprayProfile>();
        profile->Direction = direction;
        profile->Spread = spread;
        return profile;
    }
}
