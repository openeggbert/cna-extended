// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Profiles/LineProfile.hpp"

#include "CNA/Extended/FastRandom.hpp"
#include "System/InvalidOperationException.hpp"

namespace CNA::Extended::Particles::Profiles
{
    void LineProfile::GetOffsetAndHeading(Vector2* offset, Vector2* heading)
    {
        const float value = FastRandom::getSharedProperty().NextSingle(Length * -0.5f, Length * 0.5f);
        const Vector2 normalizedAxis = Vector2::Normalize(Axis);
        offset->X = normalizedAxis.X * value;
        offset->Y = normalizedAxis.Y * value;

        switch (Radiate)
        {
            case LineRadiation::None:
                FastRandom::getSharedProperty().NextUnitVector(heading);
                break;

            case LineRadiation::Directional:
            {
                const Vector2 normalizedDirection = Vector2::Normalize(Direction);
                heading->X = normalizedDirection.X;
                heading->Y = normalizedDirection.Y;
                break;
            }

            case LineRadiation::PerpendicularUp:
                heading->X = normalizedAxis.Y;
                heading->Y = -normalizedAxis.X;
                break;

            case LineRadiation::PerpendicularDown:
                heading->X = -normalizedAxis.Y;
                heading->Y = normalizedAxis.X;
                break;

            default:
                throw System::InvalidOperationException("Unsupported radiation mode");
        }
    }
}
