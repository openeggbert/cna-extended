// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/OpacityInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <algorithm>

namespace CNA::Extended::World3DEXT
{
    void OpacityInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        // A-06 (audit.md): out-of-[0,1] opacity is real UB at the render call site's
        // static_cast<std::uint8_t>(opacity * 255.0f) -- clamp here, matching the fix this
        // exact interpolation logic already had before it moved into this class.
        particle->OpacityEXT = std::clamp(StartValue + (EndValue - StartValue) * amount, 0.0f, 1.0f);
    }
}
