// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/Size2JsonConverter.cs. Despite the upstream
// filename, this converts `SizeF` -- `Size2` is a historical name for the same type, already
// ported as `CNA::Extended::SizeF` (see SizeF.hpp). See ColorJsonConverter.hpp's header comment
// for the nlohmann::adl_serializer<T> design rationale shared by every value-type converter in
// this module.
#pragma once

#include "CNA/Extended/SizeF.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<CNA::Extended::SizeF>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::SizeF& value);
        static CNA::Extended::SizeF from_json(const nlohmann::ordered_json& j);
    };
}
