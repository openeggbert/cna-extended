// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/Utf8JsonReaderExtensions.cs. Upstream reads
// from a streaming `Utf8JsonReader` (System.Text.Json's token-by-token reader); sharp-runtime's
// `JsonSerializer` is DOM-based (built on nlohmann's parsed `ordered_json` tree, not a streaming
// reader -- see JsonSerializer.hpp's own header comment), so this operates on an already-parsed
// `nlohmann::ordered_json` value instead of a reader reference. Same three supported shapes as
// upstream (JSON array, delimited string, single number), same fallback order.
//
// `ReadAsDelimitedString`'s upstream dispatch table is a closed `Dictionary<Type, Func<string,
// object>>` covering exactly three types (`int`, `float`, `HslColor`) -- not real reflection.
// Translated as explicit function-template specializations below (`ParseDelimitedElement<T>`),
// matching that same closed set; adding a fourth type upstream never happened either.
#pragma once

#include "CNA/Extended/ColorHelper.hpp"
#include "CNA/Extended/HslColor.hpp"
#include "nlohmann/json.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace CNA::Extended::Serialization::Json
{
    template <typename T>
    T ParseDelimitedElement(const std::string& text);

    template <>
    inline int ParseDelimitedElement<int>(const std::string& text)
    {
        return std::stoi(text);
    }

    template <>
    inline float ParseDelimitedElement<float>(const std::string& text)
    {
        return std::stof(text);
    }

    template <>
    inline HslColor ParseDelimitedElement<HslColor>(const std::string& text)
    {
        return HslColor::FromRgb(ColorHelper::FromHex(text));
    }

    /** @brief Splits @p value on spaces and parses each element via ParseDelimitedElement<T>. Empty input yields an empty vector. */
    template <typename T>
    std::vector<T> ReadAsDelimitedString(const std::string& value)
    {
        std::vector<T> result;
        if (value.empty())
        {
            return result;
        }

        std::istringstream stream(value);
        std::string token;
        while (stream >> token)
        {
            result.push_back(ParseDelimitedElement<T>(token));
        }
        return result;
    }

    /**
     * @brief Reads @p j as a "multi-dimensional" value: a JSON array, a space-delimited string, or a single number.
     * @throws std::invalid_argument if @p j is none of those shapes.
     */
    template <typename T>
    std::vector<T> ReadAsMultiDimensional(const nlohmann::ordered_json& j)
    {
        if (j.is_array())
        {
            std::vector<T> result;
            result.reserve(j.size());
            for (const auto& element : j)
            {
                result.push_back(element.template get<T>());
            }
            return result;
        }

        if (j.is_string())
        {
            return ReadAsDelimitedString<T>(j.template get<std::string>());
        }

        if (j.is_number())
        {
            return std::vector<T>{j.template get<T>()};
        }

        throw std::invalid_argument("is not currently supported in the multi-dimensional parser");
    }
}
