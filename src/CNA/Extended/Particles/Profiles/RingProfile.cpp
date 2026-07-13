// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/RingProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void RingProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        FastRandom::getSharedProperty().NextUnitVector(heading);

        switch (Radiate)
        {
            case CircleRadiation::In:
                offset->X = -heading->X * Radius;
                offset->Y = -heading->Y * Radius;
                break;

            case CircleRadiation::Out:
                offset->X = heading->X * Radius;
                offset->Y = heading->Y * Radius;
                break;

            case CircleRadiation::None:
                offset->X = heading->X * Radius;
                offset->Y = heading->Y * Radius;
                FastRandom::getSharedProperty().NextUnitVector(heading);
                break;

            default:
                throw System::ArgumentOutOfRangeException("Radiate", "Unsupported radiation mode");
        }
    }
}
