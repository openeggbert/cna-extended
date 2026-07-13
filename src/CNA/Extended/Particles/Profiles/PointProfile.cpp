// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/PointProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void PointProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        offset->X = 0.0f;
        offset->Y = 0.0f;
        FastRandom::getSharedProperty().NextUnitVector(heading);
    }
}
