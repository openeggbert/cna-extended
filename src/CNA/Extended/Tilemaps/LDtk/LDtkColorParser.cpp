// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/LDtk/LDtkColorParser.hpp"

namespace CNA::Extended::Tilemaps::LDtk
{
    namespace
    {
        // std::stoi(str, &pos, 16) only parses a leading run of valid hex digits and reports how
        // many characters it consumed via `pos` -- it does not throw for trailing garbage the way
        // C#'s Convert.ToInt32(str, 16) does. Checking `pos == byteHex.size()` after the call
        // reproduces that strictness (matches this session's established XmlReaderExtensions.cpp
        // fix for the same std::stoi/int.Parse strictness gap).
        bool TryParseHexByte(const std::string& byteHex, int& result)
        {
            try
            {
                std::size_t pos = 0;
                const int value = std::stoi(byteHex, &pos, 16);
                if (pos != byteHex.size())
                {
                    return false;
                }
                result = value;
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }
    }

    Color ParseColor(const std::string& colorString)
    {
        if (colorString.empty())
        {
            return Color::Transparent;
        }

        if (colorString[0] != '#')
        {
            return Color::Transparent;
        }

        const std::string hex = colorString.substr(1);

        if (hex.size() != 6)
        {
            return Color::Transparent;
        }

        int r = 0;
        int g = 0;
        int b = 0;
        if (!TryParseHexByte(hex.substr(0, 2), r) || !TryParseHexByte(hex.substr(2, 2), g) || !TryParseHexByte(hex.substr(4, 2), b))
        {
            return Color::Transparent;
        }

        return Color(r, g, b, 255);
    }

    bool TryParseColor(const std::string& colorString, Color& color)
    {
        color = ParseColor(colorString);
        return !(color == Color::Transparent) || colorString == "#000000";
    }
}
