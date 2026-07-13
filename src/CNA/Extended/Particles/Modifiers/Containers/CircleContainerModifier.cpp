// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Containers/CircleContainerModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

#include <cmath>

namespace CNA::Extended::Particles::Modifiers::Containers
{
    void CircleContainerModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        const float radiusSq = Radius * Radius;

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            Vector2 localPos;
            localPos.X = particle->Position[0] - particle->TriggeredPos[0];
            localPos.Y = particle->Position[1] - particle->TriggeredPos[1];

            const float distSq = localPos.LengthSquared();
            const Vector2 normal = Vector2::Normalize(localPos);

            if (Inside)
            {
                if (distSq < radiusSq)
                {
                    continue;
                }
                SetReflected(distSq, particle, normal);
            }
            else
            {
                if (distSq > radiusSq)
                {
                    continue;
                }
                SetReflected(distSq, particle, -normal);
            }
        }
    }

    void CircleContainerModifier::SetReflected(float distSq, Data::Particle* particle, const Vector2& normal)
    {
        const float dist = std::sqrt(distSq);
        const float d = dist - Radius;

        const float twoRestDot = 2.0f * RestitutionCoefficient * Vector2::Dot(Vector2(particle->Velocity[0], particle->Velocity[1]), normal);

        particle->Velocity[0] -= twoRestDot * normal.X;
        particle->Velocity[1] -= twoRestDot * normal.Y;

        // exact computation requires sqrt or goniometrics
        particle->Position[0] -= normal.X * d;
        particle->Position[1] -= normal.Y * d;
    }
}
