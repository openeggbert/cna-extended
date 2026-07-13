// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleContainerModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers::Containers
{
    void RectangleContainerModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        for (int i = 0; i < particleCount && iterator.getHasNextProperty(); ++i)
        {
            Data::Particle* particle = iterator.Next();

            const float left = particle->TriggeredPos[0] + static_cast<float>(Width) * -0.5f;
            const float right = particle->TriggeredPos[0] + static_cast<float>(Width) * 0.5f;
            const float top = particle->TriggeredPos[1] + static_cast<float>(Height) * -0.5f;
            const float bottom = particle->TriggeredPos[1] + static_cast<float>(Height) * 0.5f;

            float xPos = particle->Position[0];
            float xVel = particle->Velocity[0];
            float yPos = particle->Position[1];
            float yVel = particle->Velocity[1];

            // Upstream applies an inconsistent (int) truncation across these four checks (left/bottom
            // truncate, right/top don't) -- preserved exactly rather than "fixed" to be uniform.
            if (static_cast<int>(particle->Position[0]) < left)
            {
                xPos = left + (left - xPos);
                xVel = -xVel * RestitutionCoefficient;
            }
            else if (particle->Position[0] > right)
            {
                xPos = right - (xPos - right);
                xVel = -xVel * RestitutionCoefficient;
            }

            if (particle->Position[1] < top)
            {
                yPos = top + (top - yPos);
                yVel = -yVel * RestitutionCoefficient;
            }
            else if (static_cast<int>(particle->Position[1]) > bottom)
            {
                yPos = bottom - (yPos - bottom);
                yVel = -yVel * RestitutionCoefficient;
            }

            particle->Position[0] = xPos;
            particle->Position[1] = yPos;
            particle->Velocity[0] = xVel;
            particle->Velocity[1] = yVel;
        }
    }
}
