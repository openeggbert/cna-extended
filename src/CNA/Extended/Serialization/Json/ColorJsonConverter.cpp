// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/ColorJsonConverter.hpp"

#include "CNA/Extended/ColorExtensions.hpp"
#include "CNA/Extended/ColorHelper.hpp"

#include <stdexcept>

namespace nlohmann
{
    void adl_serializer<Microsoft::Xna::Framework::Color>::to_json(nlohmann::ordered_json& j, const Microsoft::Xna::Framework::Color& value)
    {
        j = CNA::Extended::ToHex(value);
    }

    Microsoft::Xna::Framework::Color adl_serializer<Microsoft::Xna::Framework::Color>::from_json(const nlohmann::ordered_json& j)
    {
        const std::string text = j.get<std::string>();
        if (text.empty())
        {
            throw std::invalid_argument("Color JSON string must not be empty.");
        }
        return text[0] == '#' ? CNA::Extended::ColorHelper::FromHex(text) : CNA::Extended::ColorHelper::FromName(text);
    }
}
