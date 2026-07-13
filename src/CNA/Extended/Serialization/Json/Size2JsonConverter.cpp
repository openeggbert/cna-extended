// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/Size2JsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"

#include <stdexcept>

namespace nlohmann
{
    void adl_serializer<CNA::Extended::SizeF>::to_json(nlohmann::ordered_json& j, const CNA::Extended::SizeF& value)
    {
        j = std::to_string(value.Width) + " " + std::to_string(value.Height);
    }

    CNA::Extended::SizeF adl_serializer<CNA::Extended::SizeF>::from_json(const nlohmann::ordered_json& j)
    {
        const std::vector<float> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<float>(j);

        if (values.size() == 2)
        {
            return CNA::Extended::SizeF(values[0], values[1]);
        }

        if (values.size() == 1)
        {
            return CNA::Extended::SizeF(values[0], values[0]);
        }

        throw std::invalid_argument("Invalid Size2 property value");
    }
}
