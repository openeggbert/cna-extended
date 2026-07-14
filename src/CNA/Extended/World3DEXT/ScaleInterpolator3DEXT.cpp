// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ScaleInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    void ScaleInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        particle->ScaleEXT.X = StartValue.X + (EndValue.X - StartValue.X) * amount;
        particle->ScaleEXT.Y = StartValue.Y + (EndValue.Y - StartValue.Y) * amount;
    }
}
