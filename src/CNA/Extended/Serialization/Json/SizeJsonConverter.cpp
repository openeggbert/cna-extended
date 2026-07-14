// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/SizeJsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"

#include <stdexcept>

namespace nlohmann
{
    void adl_serializer<CNA::Extended::Size>::to_json(nlohmann::ordered_json& j, const CNA::Extended::Size& value)
    {
        j = std::to_string(value.Width) + " " + std::to_string(value.Height);
    }

    CNA::Extended::Size adl_serializer<CNA::Extended::Size>::from_json(const nlohmann::ordered_json& j)
    {
        const std::vector<int> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<int>(j);

        if (values.size() == 2)
        {
            return CNA::Extended::Size(values[0], values[1]);
        }

        if (values.size() == 1)
        {
            return CNA::Extended::Size(values[0], values[0]);
        }

        throw std::invalid_argument("Invalid Size property value");
    }
}
