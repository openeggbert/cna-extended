// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/ColorJsonConverter.cs. Upstream is a C#
// `JsonConverter<T>` class, opted into per-call via `JsonSerializerOptions.Converters.Add(...)`.
// `sharp-runtime`'s `JsonSerializer` has no per-call converter registration -- it dispatches
// exclusively through nlohmann's global ADL customization points (see JsonSerializer.hpp's own
// header comment: this is a deliberate, documented reflection replacement, not an omission). Since
// `Color` is a foreign type this project doesn't own the definition of (it lives in `cna`'s
// `Microsoft::Xna::Framework`), free `to_json`/`from_json` functions in that type's own namespace
// aren't an option here (unlike the TexturePacker DTOs, which this project *does* own). nlohmann's
// own documented mechanism for exactly this situation is a `nlohmann::adl_serializer<T>` full
// specialization -- used here and by every other value-type converter in this module
// (HslColor/Vector2/RectangleF/Thickness/SizeF/Interval<T>). The practical effect matches
// registering the converter once and using it everywhere, which is how every real caller in this
// codebase actually uses these types today; there is no plain (non-hex-string) JSON representation
// of `Color` anywhere else in this project to conflict with.
//
// `from_json` is the return-by-value form (`static T from_json(const json&)`), not the out-param
// form (`static void from_json(const json&, T&)`) nlohmann's own examples usually show: `Color`
// has no default constructor (confirmed by reading Color.hpp -- every constructor is parameterized
// or `explicit`), and the out-param form requires nlohmann to default-construct a `T` first, which
// fails silently at the SFINAE level (not a helpful error at the actual call site) for
// non-default-constructible types. Verified empirically with a minimal repro. The return-by-value
// form works for both default- and non-default-constructible types alike, so it's used
// consistently across every converter in this module, not just for `Color`.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<Microsoft::Xna::Framework::Color>
    {
        static void to_json(nlohmann::ordered_json& j, const Microsoft::Xna::Framework::Color& value);
        static Microsoft::Xna::Framework::Color from_json(const nlohmann::ordered_json& j);
    };
}
