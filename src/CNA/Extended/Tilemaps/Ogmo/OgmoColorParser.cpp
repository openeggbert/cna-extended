// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Ogmo/OgmoColorParser.hpp"

#include <stdexcept>

namespace CNA::Extended::Tilemaps::Ogmo
{
    Color ParseColor(const std::string& colorString)
    {
        if (colorString.empty())
        {
            return Color::White;
        }

        if (colorString[0] != '#')
        {
            return Color::White;
        }

        const std::string hex = colorString.substr(1);

        if (hex.size() != 8)
        {
            return Color::White;
        }

        try
        {
            const int r = std::stoi(hex.substr(0, 2), nullptr, 16);
            const int g = std::stoi(hex.substr(2, 2), nullptr, 16);
            const int b = std::stoi(hex.substr(4, 2), nullptr, 16);
            const int a = std::stoi(hex.substr(6, 2), nullptr, 16);

            return Color(r, g, b, a);
        }
        catch (const std::invalid_argument&)
        {
            return Color::White;
        }
    }
}
