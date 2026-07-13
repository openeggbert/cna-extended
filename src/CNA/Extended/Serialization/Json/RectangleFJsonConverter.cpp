// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/RectangleFJsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"

namespace nlohmann
{
    void adl_serializer<CNA::Extended::RectangleF>::to_json(nlohmann::ordered_json& j, const CNA::Extended::RectangleF& value)
    {
        j = std::to_string(value.getLeftProperty()) + " " + std::to_string(value.getTopProperty()) + " " +
            std::to_string(value.Width) + " " + std::to_string(value.Height);
    }

    CNA::Extended::RectangleF adl_serializer<CNA::Extended::RectangleF>::from_json(const nlohmann::ordered_json& j)
    {
        const std::vector<float> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<float>(j);
        return CNA::Extended::RectangleF(values[0], values[1], values[2], values[3]);
    }
}
