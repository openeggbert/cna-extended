// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/ThicknessJsonConverter.cs. See
// ColorJsonConverter.hpp's header comment for the nlohmann::adl_serializer<T> design rationale
// shared by every value-type converter in this module.
#pragma once

#include "CNA/Extended/Thickness.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<CNA::Extended::Thickness>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::Thickness& value);
        static CNA::Extended::Thickness from_json(const nlohmann::ordered_json& j);
    };
}
