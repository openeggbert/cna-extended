// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RotationModifier3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    void RotationModifier3DEXT::Update(float elapsedSeconds, std::vector<Particle3DEXT>& particles, int startIndex, int particleCount)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const float rotationRateDelta = RotationRateEXT * elapsedSeconds;
        const int end = std::min(static_cast<int>(particles.size()), startIndex + particleCount);
        for (int i = startIndex; i < end; ++i)
        {
            particles[static_cast<std::size_t>(i)].RotationEXT += rotationRateDelta;
        }
    }
}
