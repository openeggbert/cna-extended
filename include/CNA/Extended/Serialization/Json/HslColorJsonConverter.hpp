// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/HslColorJsonConverter.cs -- delegates to the
// same hex-string representation as ColorJsonConverter (see that file's header comment for the
// nlohmann::adl_serializer<T> design rationale shared by every converter in this module).
#pragma once

#include "CNA/Extended/HslColor.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<CNA::Extended::HslColor>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::HslColor& value);
        static CNA::Extended::HslColor from_json(const nlohmann::ordered_json& j);
    };
}
