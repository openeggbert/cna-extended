// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
#include "CNA/Extended/World3DEXT/ColorInterpolator3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Particle3DEXT.hpp"

#include <cstdint>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Color;

    void ColorInterpolator3DEXT::Update(float amount, Particle3DEXT* particle)
    {
        if (!getEnabledProperty())
        {
            return;
        }

        const auto lerpChannel = [amount](std::uint8_t a, std::uint8_t b) {
            return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * amount);
        };

        particle->ColorEXT = Color(lerpChannel(StartValue.getRProperty(), EndValue.getRProperty()),
                                    lerpChannel(StartValue.getGProperty(), EndValue.getGProperty()),
                                    lerpChannel(StartValue.getBProperty(), EndValue.getBProperty()), static_cast<std::uint8_t>(255));
    }
}
