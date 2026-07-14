// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/DragModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    void DragModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            Particle3DEXT& particle = particles[static_cast<std::size_t>(i)];

            const float drag = -DragCoefficientEXT * DensityEXT * particle.MassEXT * elapsedSeconds;
            particle.VelocityEXT = particle.VelocityEXT + particle.VelocityEXT * drag;
        }
    }
}
