// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/VortexModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    VortexModifier3DEXT::VortexModifier3DEXT() : Modifier3DEXT("VortexModifier3DEXT")
    {
        setRotationAngleEXTProperty(0.0f);
    }

    void VortexModifier3DEXT::setRotationAngleEXTProperty(float value)
    {
        if (rotationAngleEXT_ == value)
        {
            return;
        }

        rotationAngleEXT_ = value;
        cosAngleEXT_ = std::cos(rotationAngleEXT_);
        sinAngleEXT_ = std::sin(rotationAngleEXT_);
    }

    void VortexModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        // A-06-style guard (audit.md): AxisEXT is a public, caller-settable field with no
        // constructor-level guarantee it stays nonzero.
        const Vector3 axis = AxisEXT.LengthSquared() > 1e-12f ? Vector3::Normalize(AxisEXT) : Vector3::Up;

        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];

            const Vector3 vortexPoint = particle.TriggeredPositionEXT + PositionEXT;
            const Vector3 d = particle.PositionEXT - vortexPoint;

            // Decompose d into the component along the axis and the component perpendicular
            // to it -- only the perpendicular component matters for a line (not point) vortex.
            const Vector3 alongAxis = Vector3::Dot(d, axis) * axis;
            const Vector3 radial = d - alongAxis;
            const float distance = radial.Length();

            if (distance < InnerRadiusEXT || distance > OuterRadiusEXT)
            {
                continue;
            }

            // Same latent div-by-zero as upstream's 2D VortexModifier when distance is exactly
            // 0 (only reachable with InnerRadiusEXT == 0 and a particle exactly on the axis
            // line) -- guarded here since this is new code, not a preservation of prior
            // behavior (see this file's header comment / other Phase 11 C-2/C-3 profiles for
            // the same established proactive-guard precedent).
            if (distance < 1e-6f)
            {
                continue;
            }

            const Vector3 gravity = radial * (-1.0f / distance);

            // Rodrigues' rotation formula restricted to a vector already perpendicular to
            // axis: v*cos(theta) + (axis x v)*sin(theta) -- generalizes 2D's 2x2 rotation
            // matrix (see this file's header comment).
            const Vector3 rotatedGravity = gravity * cosAngleEXT_ + Vector3::Cross(axis, gravity) * sinAngleEXT_;

            const float distanceRatio = OuterRadiusEXT / distance;
            const float forceStrength = StrengthEXT * distanceRatio;

            particle.VelocityEXT = particle.VelocityEXT + rotatedGravity * forceStrength * elapsedSeconds;

            const float velocityMagnitude = particle.VelocityEXT.Length();
            if (velocityMagnitude > MaxVelocityEXT)
            {
                const float scale = MaxVelocityEXT / velocityMagnitude;
                particle.VelocityEXT = particle.VelocityEXT * scale;
            }
        }
    }
}
