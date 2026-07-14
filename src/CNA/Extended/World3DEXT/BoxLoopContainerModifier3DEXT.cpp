// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BoxLoopContainerModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void BoxLoopContainerModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];

            const float left = particle.TriggeredPositionEXT.X + static_cast<float>(WidthEXT) * -0.5f;
            const float right = particle.TriggeredPositionEXT.X + static_cast<float>(WidthEXT) * 0.5f;
            const float top = particle.TriggeredPositionEXT.Y + static_cast<float>(HeightEXT) * -0.5f;
            const float bottom = particle.TriggeredPositionEXT.Y + static_cast<float>(HeightEXT) * 0.5f;
            const float front = particle.TriggeredPositionEXT.Z + static_cast<float>(DepthEXT) * -0.5f;
            const float back = particle.TriggeredPositionEXT.Z + static_cast<float>(DepthEXT) * 0.5f;

            float xPos = particle.PositionEXT.X;
            float yPos = particle.PositionEXT.Y;
            float zPos = particle.PositionEXT.Z;

            if (static_cast<int>(particle.PositionEXT.X) < left)
            {
                xPos = particle.PositionEXT.X + static_cast<float>(WidthEXT);
            }
            else if (static_cast<int>(particle.PositionEXT.X) > right)
            {
                xPos = particle.PositionEXT.X - static_cast<float>(WidthEXT);
            }

            if (static_cast<int>(particle.PositionEXT.Y) < top)
            {
                yPos = particle.PositionEXT.Y + static_cast<float>(HeightEXT);
            }
            else if (static_cast<int>(particle.PositionEXT.Y) > bottom)
            {
                yPos = particle.PositionEXT.Y - static_cast<float>(HeightEXT);
            }

            if (static_cast<int>(particle.PositionEXT.Z) < front)
            {
                zPos = particle.PositionEXT.Z + static_cast<float>(DepthEXT);
            }
            else if (static_cast<int>(particle.PositionEXT.Z) > back)
            {
                zPos = particle.PositionEXT.Z - static_cast<float>(DepthEXT);
            }

            particle.PositionEXT = Vector3(xPos, yPos, zPos);
        }
    }
}
