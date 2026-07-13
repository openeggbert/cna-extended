// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/VortexModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

#include <cmath>

namespace CNA::Extended::Particles::Modifiers
{
    VortexModifier::VortexModifier() : Modifier("VortexModifier")
    {
        setRotationAngleProperty(0.0f);
    }

    void VortexModifier::setRotationAngleProperty(float value)
    {
        if (rotationAngle_ == value)
        {
            return;
        }

        rotationAngle_ = value;

        cosAngle_ = std::cos(rotationAngle_);
        sinAngle_ = std::sin(rotationAngle_);
    }

    void VortexModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            const float vortexX = particle->TriggeredPos[0] + Position.X;
            const float vortexY = particle->TriggeredPos[1] + Position.Y;

            const float dx = particle->Position[0] - vortexX;
            const float dy = particle->Position[1] - vortexY;
            const float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < InnerRadius || distance > OuterRadius)
            {
                continue;
            }

            const float gravityX = -dx / distance;
            const float gravityY = -dy / distance;

            const float rotatedX = gravityX * cosAngle_ - gravityY * sinAngle_;
            const float rotatedY = gravityX * sinAngle_ + gravityY * cosAngle_;

            const float distanceRatio = OuterRadius / distance;
            const float forceStrength = Strength * distanceRatio;

            particle->Velocity[0] += rotatedX * forceStrength * elapsedSeconds;
            particle->Velocity[1] += rotatedY * forceStrength * elapsedSeconds;

            const float velocityMagnitude =
                std::sqrt(particle->Velocity[0] * particle->Velocity[0] + particle->Velocity[1] * particle->Velocity[1]);

            if (velocityMagnitude > MaxVelocity)
            {
                const float scale = MaxVelocity / velocityMagnitude;
                particle->Velocity[0] *= scale;
                particle->Velocity[1] *= scale;
            }
        }
    }
}
