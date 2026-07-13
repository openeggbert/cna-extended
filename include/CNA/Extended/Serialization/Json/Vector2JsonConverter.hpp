// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/Vector2JsonConverter.cs. See
// ColorJsonConverter.hpp's header comment for the nlohmann::adl_serializer<T> design rationale
// shared by every value-type converter in this module.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann
{
    template <>
    struct adl_serializer<Microsoft::Xna::Framework::Vector2>
    {
        static void to_json(nlohmann::ordered_json& j, const Microsoft::Xna::Framework::Vector2& value);
        static Microsoft::Xna::Framework::Vector2 from_json(const nlohmann::ordered_json& j);
    };
}
