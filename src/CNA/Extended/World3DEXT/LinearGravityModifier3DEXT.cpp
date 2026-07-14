// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/LinearGravityModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    void LinearGravityModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const Microsoft::Xna::Framework::Vector3 delta = GravityEXT * elapsedSeconds;
        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];
            particle.VelocityEXT = particle.VelocityEXT + delta * particle.MassEXT;
        }
    }
}
