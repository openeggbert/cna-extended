// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/CircleProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"
#include "System/ArgumentOutOfRangeException.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void CircleProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        const float distance = FastRandom::getSharedProperty().NextSingle(0.0f, Radius);

        FastRandom::getSharedProperty().NextUnitVector(heading);

        switch (Radiate)
        {
            case CircleRadiation::In:
                offset->X = -heading->X * distance;
                offset->Y = -heading->Y * distance;
                break;

            case CircleRadiation::Out:
                offset->X = heading->X * distance;
                offset->Y = heading->Y * distance;
                break;

            case CircleRadiation::None:
                offset->X = heading->X * distance;
                offset->Y = heading->Y * distance;
                FastRandom::getSharedProperty().NextUnitVector(heading);
                break;

            default:
                throw System::ArgumentOutOfRangeException("Radiate", "Unsupported radiation mode");
        }
    }
}
