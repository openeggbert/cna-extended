// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/RotationInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    void RotationInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->RotationEXT = StartValue + (EndValue - StartValue) * amount;
    }
}
