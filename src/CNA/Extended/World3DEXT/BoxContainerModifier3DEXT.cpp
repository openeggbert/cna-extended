// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/BoxContainerModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;

    void BoxContainerModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
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
            float xVel = particle.VelocityEXT.X;
            float yPos = particle.PositionEXT.Y;
            float yVel = particle.VelocityEXT.Y;
            float zPos = particle.PositionEXT.Z;
            float zVel = particle.VelocityEXT.Z;

            // X/Y: preserves 2D RectangleContainerModifier's exact inconsistent (int)
            // truncation across these checks (left/bottom truncate, right/top don't) rather
            // than "fixing" it -- see this file's header comment.
            if (static_cast<int>(particle.PositionEXT.X) < left)
            {
                xPos = left + (left - xPos);
                xVel = -xVel * RestitutionCoefficientEXT;
            }
            else if (particle.PositionEXT.X > right)
            {
                xPos = right - (xPos - right);
                xVel = -xVel * RestitutionCoefficientEXT;
            }

            if (particle.PositionEXT.Y < top)
            {
                yPos = top + (top - yPos);
                yVel = -yVel * RestitutionCoefficientEXT;
            }
            else if (static_cast<int>(particle.PositionEXT.Y) > bottom)
            {
                yPos = bottom - (yPos - bottom);
                yVel = -yVel * RestitutionCoefficientEXT;
            }

            // Z: no 2D precedent for a third axis -- consistent (non-truncating) checks on
            // both sides, not an invented new asymmetry.
            if (particle.PositionEXT.Z < front)
            {
                zPos = front + (front - zPos);
                zVel = -zVel * RestitutionCoefficientEXT;
            }
            else if (particle.PositionEXT.Z > back)
            {
                zPos = back - (zPos - back);
                zVel = -zVel * RestitutionCoefficientEXT;
            }

            particle.PositionEXT = Vector3(xPos, yPos, zPos);
            particle.VelocityEXT = Vector3(xVel, yVel, zVel);
        }
    }
}
