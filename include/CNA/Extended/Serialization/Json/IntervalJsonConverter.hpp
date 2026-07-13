// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/IntervalJsonConverter.cs (a generic
// `JsonConverter<Interval<T>>`, upstream constrained to `T : IComparable<T>`). `Interval<T>` is
// already a C++ template (Interval.hpp); this ports as a *partial* nlohmann::adl_serializer
// specialization over `T`, header-only since it's a template -- see ColorJsonConverter.hpp's
// header comment for the overall adl_serializer design rationale shared by this module.
#pragma once

#include "CNA/Extended/Interval.hpp"
#include "CNA/Extended/Serialization/Json/Utf8JsonReaderExtensions.hpp"
#include "nlohmann/json.hpp"

#include <stdexcept>

namespace nlohmann
{
    template <typename T>
    struct adl_serializer<CNA::Extended::Interval<T>>
    {
        static void to_json(nlohmann::ordered_json& j, const CNA::Extended::Interval<T>& value)
        {
            j = nlohmann::ordered_json::array({value.getMinProperty(), value.getMaxProperty()});
        }

        static CNA::Extended::Interval<T> from_json(const nlohmann::ordered_json& j)
        {
            const std::vector<T> values = CNA::Extended::Serialization::Json::ReadAsMultiDimensional<T>(j);

            if (values.size() == 2)
            {
                return values[0] < values[1] ? CNA::Extended::Interval<T>(values[0], values[1])
                                              : CNA::Extended::Interval<T>(values[1], values[0]);
            }

            if (values.size() == 1)
            {
                return CNA::Extended::Interval<T>(values[0], values[0]);
            }

            throw std::invalid_argument("Invalid interval");
        }
    };
}
