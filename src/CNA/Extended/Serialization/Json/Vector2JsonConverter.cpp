// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/Vector2JsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"

#include <stdexcept>

namespace nlohmann
{
    void adl_serializer<Microsoft::Xna::Framework::Vector2>::to_json(
        nlohmann::ordered_json& j, const Microsoft::Xna::Framework::Vector2& value)
    {
        j = std::to_string(value.X) + " " + std::to_string(value.Y);
    }

    Microsoft::Xna::Framework::Vector2 adl_serializer<Microsoft::Xna::Framework::Vector2>::from_json(const nlohmann::ordered_json& j)
    {
        const std::vector<float> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<float>(j);

        if (values.size() == 2)
        {
            return Microsoft::Xna::Framework::Vector2(values[0], values[1]);
        }

        if (values.size() == 1)
        {
            return Microsoft::Xna::Framework::Vector2(values[0]);
        }

        throw std::invalid_argument("Invalid Vector2 property value");
    }
}
