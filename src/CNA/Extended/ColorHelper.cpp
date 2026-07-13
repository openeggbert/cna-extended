// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ColorHelper.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace CNA::Extended
{
    namespace
    {
        std::string ToLowerAscii(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        // Upstream builds this table via reflection over Color's static properties
        // (typeof(Color).GetRuntimeProperties()); C++ has no runtime reflection, so this is a
        // hand-written (script-generated from CNA's Color.hpp, all 141 entries) equivalent. Keys
        // are lower-cased so FromName's lookup matches upstream's
        // StringComparer.OrdinalIgnoreCase behavior.
        const std::unordered_map<std::string, Color>& ColorsByName()
        {
            static const std::unordered_map<std::string, Color> table = {
        {"transparent", Color::Transparent},
        {"aliceblue", Color::AliceBlue},
        {"antiquewhite", Color::AntiqueWhite},
        {"aqua", Color::Aqua},
        {"aquamarine", Color::Aquamarine},
        {"azure", Color::Azure},
        {"beige", Color::Beige},
        {"bisque", Color::Bisque},
        {"black", Color::Black},
        {"blanchedalmond", Color::BlanchedAlmond},
        {"blue", Color::Blue},
        {"blueviolet", Color::BlueViolet},
        {"brown", Color::Brown},
        {"burlywood", Color::BurlyWood},
        {"cadetblue", Color::CadetBlue},
        {"chartreuse", Color::Chartreuse},
        {"chocolate", Color::Chocolate},
        {"coral", Color::Coral},
        {"cornflowerblue", Color::CornflowerBlue},
        {"cornsilk", Color::Cornsilk},
        {"crimson", Color::Crimson},
        {"cyan", Color::Cyan},
        {"darkblue", Color::DarkBlue},
        {"darkcyan", Color::DarkCyan},
        {"darkgoldenrod", Color::DarkGoldenrod},
        {"darkgray", Color::DarkGray},
        {"darkgreen", Color::DarkGreen},
        {"darkkhaki", Color::DarkKhaki},
        {"darkmagenta", Color::DarkMagenta},
        {"darkolivegreen", Color::DarkOliveGreen},
        {"darkorange", Color::DarkOrange},
        {"darkorchid", Color::DarkOrchid},
        {"darkred", Color::DarkRed},
        {"darksalmon", Color::DarkSalmon},
        {"darkseagreen", Color::DarkSeaGreen},
        {"darkslateblue", Color::DarkSlateBlue},
        {"darkslategray", Color::DarkSlateGray},
        {"darkturquoise", Color::DarkTurquoise},
        {"darkviolet", Color::DarkViolet},
        {"deeppink", Color::DeepPink},
        {"deepskyblue", Color::DeepSkyBlue},
        {"dimgray", Color::DimGray},
        {"dodgerblue", Color::DodgerBlue},
        {"firebrick", Color::Firebrick},
        {"floralwhite", Color::FloralWhite},
        {"forestgreen", Color::ForestGreen},
        {"fuchsia", Color::Fuchsia},
        {"gainsboro", Color::Gainsboro},
        {"ghostwhite", Color::GhostWhite},
        {"gold", Color::Gold},
        {"goldenrod", Color::Goldenrod},
        {"gray", Color::Gray},
        {"green", Color::Green},
        {"greenyellow", Color::GreenYellow},
        {"honeydew", Color::Honeydew},
        {"hotpink", Color::HotPink},
        {"indianred", Color::IndianRed},
        {"indigo", Color::Indigo},
        {"ivory", Color::Ivory},
        {"khaki", Color::Khaki},
        {"lavender", Color::Lavender},
        {"lavenderblush", Color::LavenderBlush},
        {"lawngreen", Color::LawnGreen},
        {"lemonchiffon", Color::LemonChiffon},
        {"lightblue", Color::LightBlue},
        {"lightcoral", Color::LightCoral},
        {"lightcyan", Color::LightCyan},
        {"lightgoldenrodyellow", Color::LightGoldenrodYellow},
        {"lightgray", Color::LightGray},
        {"lightgreen", Color::LightGreen},
        {"lightpink", Color::LightPink},
        {"lightsalmon", Color::LightSalmon},
        {"lightseagreen", Color::LightSeaGreen},
        {"lightskyblue", Color::LightSkyBlue},
        {"lightslategray", Color::LightSlateGray},
        {"lightsteelblue", Color::LightSteelBlue},
        {"lightyellow", Color::LightYellow},
        {"lime", Color::Lime},
        {"limegreen", Color::LimeGreen},
        {"linen", Color::Linen},
        {"magenta", Color::Magenta},
        {"maroon", Color::Maroon},
        {"mediumaquamarine", Color::MediumAquamarine},
        {"mediumblue", Color::MediumBlue},
        {"mediumorchid", Color::MediumOrchid},
        {"mediumpurple", Color::MediumPurple},
        {"mediumseagreen", Color::MediumSeaGreen},
        {"mediumslateblue", Color::MediumSlateBlue},
        {"mediumspringgreen", Color::MediumSpringGreen},
        {"mediumturquoise", Color::MediumTurquoise},
        {"mediumvioletred", Color::MediumVioletRed},
        {"midnightblue", Color::MidnightBlue},
        {"mintcream", Color::MintCream},
        {"mistyrose", Color::MistyRose},
        {"moccasin", Color::Moccasin},
        {"navajowhite", Color::NavajoWhite},
        {"navy", Color::Navy},
        {"oldlace", Color::OldLace},
        {"olive", Color::Olive},
        {"olivedrab", Color::OliveDrab},
        {"orange", Color::Orange},
        {"orangered", Color::OrangeRed},
        {"orchid", Color::Orchid},
        {"palegoldenrod", Color::PaleGoldenrod},
        {"palegreen", Color::PaleGreen},
        {"paleturquoise", Color::PaleTurquoise},
        {"palevioletred", Color::PaleVioletRed},
        {"papayawhip", Color::PapayaWhip},
        {"peachpuff", Color::PeachPuff},
        {"peru", Color::Peru},
        {"pink", Color::Pink},
        {"plum", Color::Plum},
        {"powderblue", Color::PowderBlue},
        {"purple", Color::Purple},
        {"red", Color::Red},
        {"rosybrown", Color::RosyBrown},
        {"royalblue", Color::RoyalBlue},
        {"saddlebrown", Color::SaddleBrown},
        {"salmon", Color::Salmon},
        {"sandybrown", Color::SandyBrown},
        {"seagreen", Color::SeaGreen},
        {"seashell", Color::SeaShell},
        {"sienna", Color::Sienna},
        {"silver", Color::Silver},
        {"skyblue", Color::SkyBlue},
        {"slateblue", Color::SlateBlue},
        {"slategray", Color::SlateGray},
        {"snow", Color::Snow},
        {"springgreen", Color::SpringGreen},
        {"steelblue", Color::SteelBlue},
        {"tan", Color::Tan},
        {"teal", Color::Teal},
        {"thistle", Color::Thistle},
        {"tomato", Color::Tomato},
        {"turquoise", Color::Turquoise},
        {"violet", Color::Violet},
        {"wheat", Color::Wheat},
        {"white", Color::White},
        {"whitesmoke", Color::WhiteSmoke},
        {"yellow", Color::Yellow},
        {"yellowgreen", Color::YellowGreen},
            };
            return table;
        }
    }

    Color ColorHelper::FromHex(const std::string& value)
    {
        if (value.empty())
        {
            return Color::Transparent;
        }

        std::string_view hex = value;
        if (hex.front() == '#')
        {
            hex = hex.substr(1);
        }

        std::uint32_t hexInt = 0;
        const auto result = std::from_chars(hex.data(), hex.data() + hex.size(), hexInt, 16);
        if (result.ec != std::errc() || result.ptr != hex.data() + hex.size())
        {
            throw std::invalid_argument("Malformed hexadecimal color: " + value);
        }

        int r, g, b, a;
        switch (hex.size())
        {
            case 6:
                r = static_cast<int>((hexInt & 0x00FF0000) >> 16);
                g = static_cast<int>((hexInt & 0x0000FF00) >> 8);
                b = static_cast<int>(hexInt & 0x000000FF);
                a = 255;
                break;

            case 8:
                r = static_cast<int>((hexInt & 0xFF000000) >> 24);
                g = static_cast<int>((hexInt & 0x00FF0000) >> 16);
                b = static_cast<int>((hexInt & 0x0000FF00) >> 8);
                a = static_cast<int>(hexInt & 0x000000FF);
                break;

            case 3:
                r = static_cast<int>(((hexInt & 0x00000F00) | (hexInt & 0x00000F00) << 4) >> 8);
                g = static_cast<int>(((hexInt & 0x000000F0) | (hexInt & 0x000000F0) << 4) >> 4);
                b = static_cast<int>((hexInt & 0x0000000F) | (hexInt & 0x0000000F) << 4);
                a = 255;
                break;

            case 4:
                r = static_cast<int>(((hexInt & 0x0000F000) | (hexInt & 0x0000F000) << 4) >> 12);
                g = static_cast<int>(((hexInt & 0x00000F00) | (hexInt & 0x00000F00) << 4) >> 8);
                b = static_cast<int>(((hexInt & 0x000000F0) | (hexInt & 0x000000F0) << 4) >> 4);
                a = static_cast<int>((hexInt & 0x0000000F) | (hexInt & 0x0000000F) << 4);
                break;

            default:
                throw std::invalid_argument("Malformed hexadecimal color: " + value);
        }

        return Color(r, g, b, a);
    }

    Color ColorHelper::FromName(const std::string& name)
    {
        const auto& table = ColorsByName();
        const auto it = table.find(ToLowerAscii(name));
        if (it != table.end())
        {
            return it->second;
        }

        throw std::invalid_argument(name + " is not a valid color");
    }

    Color ColorHelper::FromAbgr(const std::uint32_t abgr)
    {
        // Upstream packs these bytes into a single RGBA uint and constructs Color from that
        // packed value. CNA's equivalent Color(UInt32) constructor is private (packed-value
        // construction is an internal detail there), so the bytes are extracted directly and
        // passed to the public Color(r, g, b, a) constructor instead -- same resulting color,
        // without needing the packed intermediate value at all.
        const auto a = static_cast<int>(abgr & 0x000000FFu);
        const auto b = static_cast<int>((abgr & 0x0000FF00u) >> 8);
        const auto g = static_cast<int>((abgr & 0x00FF0000u) >> 16);
        const auto r = static_cast<int>((abgr & 0xFF000000u) >> 24);

        return Color(r, g, b, a);
    }
}
