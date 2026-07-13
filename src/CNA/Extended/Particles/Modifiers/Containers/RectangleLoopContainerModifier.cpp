// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Particles/Modifiers/Containers/RectangleLoopContainerModifier.hpp"

#include "CNA/Extended/Particles/Data/Particle.hpp"
#include "CNA/Extended/Particles/ParticleIterator.hpp"

namespace CNA::Extended::Particles::Modifiers::Containers
{
    void RectangleLoopContainerModifier::Update(float elapsedSeconds, ParticleIterator& iterator, int particleCount)
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
            float yPos = particle->Position[1];

            if (static_cast<int>(particle->Position[0]) < left)
            {
                xPos = particle->Position[0] + static_cast<float>(Width);
            }
            else if (static_cast<int>(particle->Position[0]) > right)
            {
                xPos = particle->Position[0] - static_cast<float>(Width);
            }

            if (static_cast<int>(particle->Position[1]) < top)
            {
                yPos = particle->Position[1] + static_cast<float>(Height);
            }
            else if (static_cast<int>(particle->Position[1]) > bottom)
            {
                yPos = particle->Position[1] - static_cast<float>(Height);
            }

            particle->Position[0] = xPos;
            particle->Position[1] = yPos;
        }
    }
}
