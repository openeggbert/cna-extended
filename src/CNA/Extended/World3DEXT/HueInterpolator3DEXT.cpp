// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/HueInterpolator3DEXT.hpp"

#include "CNA/Extended/HslColor.hpp"
#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

namespace CNA::Extended::World3DEXT
{
    void HueInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const HslColor current = HslColor::FromRgb(particle->ColorEXT);
        const float hue = StartValue + (EndValue - StartValue) * amount;
        particle->ColorEXT = HslColor::ToRgb(HslColor(hue, current.getSProperty(), current.getLProperty()));
    }
}
