// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/FloatStringConverter.cs. Unlike the other
// converters in this module, this is NOT ported as an `nlohmann::adl_serializer<float>`
// specialization: `float` is a fundamental arithmetic type that nlohmann's `basic_json` handles
// through dedicated fast-path code before ever consulting `adl_serializer` for it, so a global
// specialization would silently never be invoked (and, if it somehow were honored by a future
// nlohmann version, would dangerously override JSON handling for every `float` in this entire
// codebase, not just fields meant to tolerate a string-or-number encoding). Upstream's own
// `CanConvert` already signals this is meant for selective, per-field use
// (`typeToConvert == typeof(float) || typeToConvert == typeof(string)` -- a field whose declared
// type might be serialized as either), not a blanket default. Ported instead as explicit free
// functions a caller opts into for a specific field, matching that selective intent.
#pragma once

#include "nlohmann/json.hpp"

namespace CNA::Extended::Serialization::Json
{
    /**
     * @brief Reads @p j as a float, accepting either a JSON number or a JSON string containing a number.
     * @throws nlohmann::json::exception if @p j is neither.
     */
    [[nodiscard]] float ReadFloatString(const nlohmann::ordered_json& j);

    /** @brief Writes @p value as a JSON number. */
    void WriteFloatString(nlohmann::ordered_json& j, float value);
}
