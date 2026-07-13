// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapProperties.hpp"

namespace CNA::Extended::Tilemaps
{
    std::string TilemapProperties::GetString(const std::string& key, const std::string& defaultValue) const
    {
        TilemapPropertyValue value;
        if (!properties_.TryGetValue(key, value) || value.getTypeProperty() != TilemapPropertyType::String)
        {
            return defaultValue;
        }
        return value.AsString();
    }

    int TilemapProperties::GetInt(const std::string& key, int defaultValue) const
    {
        TilemapPropertyValue value;
        if (!properties_.TryGetValue(key, value) || value.getTypeProperty() != TilemapPropertyType::Int)
        {
            return defaultValue;
        }
        return value.AsInt();
    }

    float TilemapProperties::GetFloat(const std::string& key, float defaultValue) const
    {
        TilemapPropertyValue value;
        if (!properties_.TryGetValue(key, value) || value.getTypeProperty() != TilemapPropertyType::Float)
        {
            return defaultValue;
        }
        return value.AsFloat();
    }

    bool TilemapProperties::GetBool(const std::string& key, bool defaultValue) const
    {
        TilemapPropertyValue value;
        if (!properties_.TryGetValue(key, value) || value.getTypeProperty() != TilemapPropertyType::Bool)
        {
            return defaultValue;
        }
        return value.AsBool();
    }

    Color TilemapProperties::GetColor(const std::string& key, const std::optional<Color>& defaultValue) const
    {
        const Color effectiveDefault = defaultValue.value_or(Color::White);

        TilemapPropertyValue value;
        if (!properties_.TryGetValue(key, value) || value.getTypeProperty() != TilemapPropertyType::Color)
        {
            return effectiveDefault;
        }
        return value.AsColor();
    }
}
