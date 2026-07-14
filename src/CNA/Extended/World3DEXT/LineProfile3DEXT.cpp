// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/LineProfile3DEXT.hpp"

#include "CNA/Extended/World3DEXT/RandomVector3EXT.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Random.hpp"

#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void LineProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        const float value = LengthEXT * -0.5f + random.NextSingle() * LengthEXT;
        // A-06-style guard (audit.md): AxisEXT/DirectionEXT are public, caller-settable fields
        // with no constructor-level guarantee they stay nonzero -- Vector3::Normalize of a
        // zero vector is undefined/NaN. This is brand-new code (no pre-existing 3D behavior to
        // preserve), so the guard is applied proactively rather than reproducing 2D
        // LineProfile's own unguarded Vector2::Normalize(Axis).
        const Vector3 normalizedAxis = AxisEXT.LengthSquared() > 1e-12f ? Vector3::Normalize(AxisEXT) : Vector3::Up;
        *offset = normalizedAxis * value;

        switch (RadiateEXT)
        {
            case LineRadiation3DEXT::None:
                NextUnitVector3EXT(random, *heading);
                break;

            case LineRadiation3DEXT::Directional:
            {
                *heading = DirectionEXT.LengthSquared() > 1e-12f ? Vector3::Normalize(DirectionEXT) : Vector3::Up;
                break;
            }

            case LineRadiation3DEXT::Perpendicular:
            {
                // Build an arbitrary orthonormal basis for the plane perpendicular to
                // normalizedAxis, then pick a uniformly random direction within that plane
                // (see LineRadiation3DEXT.hpp for why this collapses 2D's Up/Down pair).
                const Vector3 arbitrary = std::abs(normalizedAxis.Y) < 0.99f ? Vector3::Up : Vector3::Right;
                const Vector3 right = Vector3::Normalize(Vector3::Cross(arbitrary, normalizedAxis));
                const Vector3 up = Vector3::Cross(normalizedAxis, right);
                const float angle = random.NextSingle() * 6.28318530717958647692f; // 2*pi
                *heading = right * std::cos(angle) + up * std::sin(angle);
                break;
            }

            default:
                throw System::InvalidOperationException("Unsupported radiation mode");
        }
    }
}
