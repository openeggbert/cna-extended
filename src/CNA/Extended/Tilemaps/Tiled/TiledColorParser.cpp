// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Tiled/TiledColorParser.hpp"

#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace CNA::Extended::Tilemaps::Tiled
{
    using Parsers::TilemapParseException;

    namespace
    {
        bool IsWhitespaceOnly(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
        }

        int ParseHexByte(const std::string& colorString, const std::string& hex, std::size_t offset)
        {
            try
            {
                const std::string byteStr = hex.substr(offset, 2);
                std::size_t consumed = 0;
                const int value = std::stoi(byteStr, &consumed, 16);
                if (consumed != 2)
                {
                    throw std::invalid_argument("not exactly 2 hex digits");
                }
                return value;
            }
            catch (const std::exception&)
            {
                throw TilemapParseException(
                    "Invalid color format: '" + colorString + "'. Failed to parse hex digits");
            }
        }
    }

    std::optional<Color> Parse(const std::string& colorString)
    {
        if (colorString.empty() || IsWhitespaceOnly(colorString))
        {
            return std::nullopt;
        }

        if (colorString[0] != '#')
        {
            throw TilemapParseException(
                "Invalid color format: '" + colorString + "'. Expected format: #RRGGBB or #AARRGGBB");
        }

        const std::string hex = colorString.substr(1);

        if (hex.size() == 6)
        {
            const int r = ParseHexByte(colorString, hex, 0);
            const int g = ParseHexByte(colorString, hex, 2);
            const int b = ParseHexByte(colorString, hex, 4);
            return Color(r, g, b, 255);
        }

        if (hex.size() == 8)
        {
            const int a = ParseHexByte(colorString, hex, 0);
            const int r = ParseHexByte(colorString, hex, 2);
            const int g = ParseHexByte(colorString, hex, 4);
            const int b = ParseHexByte(colorString, hex, 6);
            return Color(r, g, b, a);
        }

        throw TilemapParseException(
            "Invalid color format: '" + colorString + "'. Expected 6 or 8 hex digits after #");
    }

    bool TryParse(const std::string& colorString, std::optional<Color>& color)
    {
        try
        {
            color = Parse(colorString);
            return true;
        }
        catch (const std::exception&)
        {
            color = std::nullopt;
            return false;
        }
    }
}
