// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/MonoGameJsonSerializerOptionsProvider.cs.
// Upstream's whole purpose is assembling a `JsonSerializerOptions.Converters` list (Interval<int>,
// Interval<float>, Interval<HslColor>, Thickness, RectangleF, TextureAtlas, SizeF converters) for a
// single deserialization call. `sharp-runtime`'s `JsonSerializer` has no per-call `Converters`
// list -- the value-type converters in this module (Interval<T>/Thickness/RectangleF/SizeF/etc.)
// are already globally active via `nlohmann::adl_serializer<T>` specializations the moment their
// headers are included (see ColorJsonConverter.hpp's header comment), so there is nothing left to
// "register" for them. `TextureAtlasJsonConverter` is the one genuinely stateful exception (needs
// a ContentManager + path) and cannot be made global this way -- callers must construct and invoke
// it directly (see TextureAtlasJsonConverter.hpp). This function is kept only for the formatting
// options (indentation, camelCase property names) it actually controls; it does not return
// anything resembling a converter list, since there is nothing left for it to build.
#pragma once

#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/Text/Json/JsonSerializerOptions.hpp"

#include <string>

namespace CNA::Extended::Serialization::Json
{
    using Microsoft::Xna::Framework::Content::ContentManager;

    /** @brief Returns JsonSerializerOptions matching upstream's formatting choices (indented, camelCase property names). */
    [[nodiscard]] System::Text::Json::JsonSerializerOptions GetOptions(ContentManager& contentManager, const std::string& contentPath);
}
