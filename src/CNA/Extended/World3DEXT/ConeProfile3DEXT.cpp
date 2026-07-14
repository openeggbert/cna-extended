// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ConeProfile3DEXT.hpp"

#include "System/Random.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void ConeProfile3DEXT::GetOffsetAndHeading(Vector3* offset, Vector3* heading, System::Random& random)
    {
        offset->X = offset->Y = offset->Z = 0.0f;

        const float halfAngle = std::clamp(HalfAngleEXT, 0.0f, 3.14159265358979323846f);
        const float minCosTheta = std::cos(halfAngle);
        const float cosTheta = minCosTheta + random.NextSingle() * (1.0f - minCosTheta);
        const float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
        const float phi = random.NextSingle() * 6.28318530717958647692f; // 2*pi

        const Vector3 localDirection(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

        // Audit finding A-06 (audit.md), independently re-verified: Vector3::Normalize of a
        // zero (or near-zero) vector is undefined/NaN, and DirectionEXT is a public,
        // caller-settable field with no constructor-level guarantee it stays nonzero. Fall
        // back to the field's own documented default (Vector3::Up) rather than propagate NaN
        // -- matches the guard originally added directly to ParticleEmitter3DEXT's own
        // SampleConeDirectionEXT before this profile existed.
        const Vector3 forward = DirectionEXT.LengthSquared() > 1e-12f ? Vector3::Normalize(DirectionEXT) : Vector3::Up;
        const Vector3 arbitrary = std::abs(forward.Y) < 0.99f ? Vector3::Up : Vector3::Right;
        const Vector3 right = Vector3::Normalize(Vector3::Cross(arbitrary, forward));
        const Vector3 up = Vector3::Cross(forward, right);

        *heading = right * localDirection.X + up * localDirection.Y + forward * localDirection.Z;
    }
}
