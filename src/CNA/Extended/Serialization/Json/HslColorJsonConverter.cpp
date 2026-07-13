// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/HslColorJsonConverter.hpp"

#include "CNA/Extended/Serialization/Json/ColorJsonConverter.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

namespace nlohmann
{
    void adl_serializer<CNA::Extended::HslColor>::to_json(nlohmann::ordered_json& j, const CNA::Extended::HslColor& value)
    {
        j = CNA::Extended::HslColor::ToRgb(value);
    }

    CNA::Extended::HslColor adl_serializer<CNA::Extended::HslColor>::from_json(const nlohmann::ordered_json& j)
    {
        const auto color = j.get<Microsoft::Xna::Framework::Color>();
        return CNA::Extended::HslColor::FromRgb(color);
    }
}
