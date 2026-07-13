// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/RectangleFJsonConverter.cs. See
// ColorJsonConverter.hpp's header comment for the nlohmann::adl_serializer<T> design rationale
// shared by every value-type converter in this module.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<CNA::Extended::RectangleF>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::RectangleF& value);
        static CNA::Extended::RectangleF from_json(const nlohmann::ordered_json& j);
    };
}
