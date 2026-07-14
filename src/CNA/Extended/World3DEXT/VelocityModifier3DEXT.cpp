// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/VelocityModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>
#include <cmath>

namespace CNA::Extended::World3DEXT
{
    void VelocityModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        (void)elapsedSeconds;
        if (!getEnabledProperty())
        {
            return;
        }

        const float velocityThreshold2 = VelocityThresholdEXT * VelocityThresholdEXT;
        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];
            const float velocitySquared = particle.VelocityEXT.LengthSquared();

            if (velocitySquared >= velocityThreshold2)
            {
                for (const std::unique_ptr<Interpolator3DEXT>& interpolator : interpolators_)
                {
                    interpolator->Update(1.0f, &particle);
                }
            }
            else
            {
                const float t = std::sqrt(velocitySquared) / VelocityThresholdEXT;
                for (const std::unique_ptr<Interpolator3DEXT>& interpolator : interpolators_)
                {
                    interpolator->Update(t, &particle);
                }
            }
        }
    }
}
