// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/SphereContainerModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void SphereContainerModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        const float radiusSq = RadiusEXT * RadiusEXT;
        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];

            const Vector3 localPos = particle.PositionEXT - particle.TriggeredPositionEXT;
            const float distSq = localPos.LengthSquared();

            // A-06-style guard (audit.md): a freshly-emitted particle from a Profile with zero
            // offset (e.g. PointProfile3DEXT/ConeProfile3DEXT) starts with PositionEXT exactly
            // equal to TriggeredPositionEXT -- localPos is the zero vector, and
            // Vector3::Normalize(Vector3::Zero) is undefined/NaN. 2D's CircleContainerModifier
            // has this same latent issue unguarded; this is new code with no prior 3D behavior
            // to preserve, so treat "exactly at center" as needing no reflection this frame.
            if (distSq < 1e-12f)
            {
                continue;
            }

            const Vector3 normal = Vector3::Normalize(localPos);

            if (InsideEXT)
            {
                if (distSq < radiusSq)
                {
                    continue;
                }
                SetReflectedEXT(distSq, particle, normal);
            }
            else
            {
                if (distSq > radiusSq)
                {
                    continue;
                }
                SetReflectedEXT(distSq, particle, normal * -1.0f);
            }
        }
    }

    void SphereContainerModifier3DEXT::SetReflectedEXT(float distSq, Particle3DEXT& particle, const Vector3& normal)
    {
        const float dist = std::sqrt(distSq);
        const float d = dist - RadiusEXT;

        const float twoRestDot = 2.0f * RestitutionCoefficientEXT * Vector3::Dot(particle.VelocityEXT, normal);
        particle.VelocityEXT = particle.VelocityEXT - normal * twoRestDot;
        particle.PositionEXT = particle.PositionEXT - normal * d;
    }
}
