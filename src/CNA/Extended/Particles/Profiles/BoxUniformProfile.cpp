// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/BoxUniformProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void BoxUniformProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        const auto perimeter = static_cast<int>(2 * Width + 2 * Height);
        const int value = FastRandom::getSharedProperty().Next(perimeter);

        if (static_cast<float>(value) < Width) // Top
        {
            offset->X = FastRandom::getSharedProperty().NextSingle(Width * -0.5f, Width * 0.5f);
            offset->Y = Height * -0.5f;
        }
        else if (static_cast<float>(value) < 2 * Width) // Bottom
        {
            offset->X = FastRandom::getSharedProperty().NextSingle(Width * -0.5f, Width * 0.5f);
            offset->Y = Height * 0.5f;
        }
        else if (static_cast<float>(value) < 2 * Width + Height) // Left
        {
            offset->X = Width * -0.5f;
            offset->Y = FastRandom::getSharedProperty().NextSingle(Height * -0.5f, Height * 0.5f);
        }
        else // Right
        {
            offset->X = Width * 0.5f;
            offset->Y = FastRandom::getSharedProperty().NextSingle(Height * -0.5f, Height * 0.5f);
        }

        FastRandom::getSharedProperty().NextUnitVector(heading);
    }
}
