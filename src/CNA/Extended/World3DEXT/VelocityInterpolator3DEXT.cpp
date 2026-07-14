// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/VelocityInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    void VelocityInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->VelocityEXT.X = StartValue.X + (EndValue.X - StartValue.X) * amount;
        particle->VelocityEXT.Y = StartValue.Y + (EndValue.Y - StartValue.Y) * amount;
        particle->VelocityEXT.Z = StartValue.Z + (EndValue.Z - StartValue.Z) * amount;
    }
}
