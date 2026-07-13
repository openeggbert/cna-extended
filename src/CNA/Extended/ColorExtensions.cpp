// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ColorExtensions.hpp"

#include <iomanip>
#include <sstream>

namespace CNA::Extended
{
    std::string ToHex(const Color& color)
    {
        std::ostringstream stream;
        stream << '#' << std::hex << std::setfill('0')
               << std::setw(2) << static_cast<int>(color.getRProperty())
               << std::setw(2) << static_cast<int>(color.getGProperty())
               << std::setw(2) << static_cast<int>(color.getBProperty())
               << std::setw(2) << static_cast<int>(color.getAProperty());
        return stream.str();
    }
}
