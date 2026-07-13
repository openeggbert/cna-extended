// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Json/FloatStringConverter.hpp"

#include "System/Single.hpp"

#include <stdexcept>

namespace CNA::Extended::Serialization::Json
{
    float ReadFloatString(const nlohmann::ordered_json& j)
    {
        // Matches upstream's `float.TryParse(...)` (not `float.Parse`) -- a string that fails to
        // parse falls through to the generic error below rather than propagating a parse
        // exception, and (via System::Single::TryParse, not std::stof) the whole string must be a
        // valid float: std::stof only parses a leading numeric prefix and silently ignores
        // trailing garbage (e.g. std::stof("1.5abc") == 1.5f, no failure).
        if (j.is_string())
        {
            float value = 0.0f;
            if (System::Single::TryParse(j.get<std::string>(), value))
            {
                return value;
            }
        }
        else if (j.is_number())
        {
            return j.get<float>();
        }

        throw std::invalid_argument("Unable to convert value to float");
    }

    void WriteFloatString(nlohmann::ordered_json& j, float value)
    {
        j = value;
    }
}
