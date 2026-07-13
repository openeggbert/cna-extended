// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/BoxProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void BoxProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        switch (FastRandom::getSharedProperty().Next(4))
        {
            case 0: // Left
                offset->X = Width * -0.5f;
                offset->Y = FastRandom::getSharedProperty().NextSingle(Height * -0.5f, Height * 0.5f);
                break;

            case 1: // Top
                offset->X = FastRandom::getSharedProperty().NextSingle(Width * -0.5f, Width * 0.5f);
                offset->Y = Height * -0.5f;
                break;

            case 2: // Right
                offset->X = Width * 0.5f;
                offset->Y = FastRandom::getSharedProperty().NextSingle(Height * -0.5f, Height * 0.5f);
                break;

            default: // Bottom
                offset->X = FastRandom::getSharedProperty().NextSingle(Width * -0.5f, Width * 0.5f);
                offset->Y = Height * 0.5f;
                break;
        }

        FastRandom::getSharedProperty().NextUnitVector(heading);
    }
}
