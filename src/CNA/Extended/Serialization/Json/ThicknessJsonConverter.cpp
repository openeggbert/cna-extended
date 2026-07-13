// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/ThicknessJsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"

namespace nlohmann
{
    void adl_serializer<CNA::Extended::Thickness>::to_json(nlohmann::ordered_json& j, const CNA::Extended::Thickness& value)
    {
        j = std::to_string(value.getLeftProperty()) + " " + std::to_string(value.getTopProperty()) + " " +
            std::to_string(value.getRightProperty()) + " " + std::to_string(value.getBottomProperty());
    }

    CNA::Extended::Thickness adl_serializer<CNA::Extended::Thickness>::from_json(const nlohmann::ordered_json& j)
    {
        const std::vector<SharpRuntime::intcs> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<SharpRuntime::intcs>(j);
        return CNA::Extended::Thickness::FromValues(values);
    }
}
