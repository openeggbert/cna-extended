// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/BoxFillProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void BoxFillProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        offset->X = FastRandom::getSharedProperty().NextSingle(Width * -0.5f, Width * 0.5f);
        offset->Y = FastRandom::getSharedProperty().NextSingle(Height * -0.5f, Height * 0.5f);
        FastRandom::getSharedProperty().NextUnitVector(heading);
    }
}
