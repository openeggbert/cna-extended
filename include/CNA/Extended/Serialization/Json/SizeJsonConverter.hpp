// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/SizeJsonConverter.cs -- converts the
// integer `Size` type (Width/Height ints), distinct from `Size2JsonConverter.cs`/
// `Size2JsonConverter.hpp` which converts `SizeF`. See ColorJsonConverter.hpp's header comment
// for the nlohmann::adl_serializer<T> design rationale shared by every value-type converter in
// this module.
#pragma once

#include "CNA/Extended/Size.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<CNA::Extended::Size>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::Size& value);
        static CNA::Extended::Size from_json(const nlohmann::ordered_json& j);
    };
}
