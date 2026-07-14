// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    float Particle3DEXT::GetAgeRatioEXT() const
    {
        if (LifetimeEXT <= 0.0f)
        {
            return 1.0f;
        }
        return std::clamp(AgeEXT / LifetimeEXT, 0.0f, 1.0f);
    }
}
