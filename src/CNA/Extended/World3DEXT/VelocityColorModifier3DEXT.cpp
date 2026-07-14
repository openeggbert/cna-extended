// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/VelocityColorModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;

    void VelocityColorModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
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
                particle.ColorEXT = VelocityColorEXT;
            }
            else
            {
                const float t = std::sqrt(velocitySquared) / VelocityThresholdEXT;
                const auto lerpChannel = [t](std::uint8_t a, std::uint8_t b) {
                    return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t);
                };

                particle.ColorEXT = Color(lerpChannel(StationaryColorEXT.getRProperty(), VelocityColorEXT.getRProperty()),
                                           lerpChannel(StationaryColorEXT.getGProperty(), VelocityColorEXT.getGProperty()),
                                           lerpChannel(StationaryColorEXT.getBProperty(), VelocityColorEXT.getBProperty()),
                                           static_cast<std::uint8_t>(255));
            }
        }
    }
}
