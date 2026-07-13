// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/SprayProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"

#include <cmath>

namespace CNA::Extended::Particles::Profiles
{
    void SprayProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        offset->X = offset->Y = 0.0f;

        float angle = std::atan2(Direction.Y, Direction.X);
        angle = FastRandom::getSharedProperty().NextSingle(angle - Spread * 0.5f, angle + Spread * 0.5f);

        heading->X = std::cos(angle);
        heading->Y = std::sin(angle);
    }
}
