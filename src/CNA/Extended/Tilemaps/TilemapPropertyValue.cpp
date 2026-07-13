// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/TilemapPropertyValue.hpp"

#include "System/InvalidOperationException.hpp"

#include <utility>

namespace CNA::Extended::Tilemaps
{
    TilemapPropertyValue::TilemapPropertyValue(TilemapPropertyType type, ValueData valueData, std::string stringValue)
        : type_(type), valueData_(valueData), stringValue_(std::move(stringValue))
    {
    }

    TilemapPropertyValue TilemapPropertyValue::CreateString(std::string value)
    {
        return TilemapPropertyValue(TilemapPropertyType::String, ValueData(), std::move(value));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateInt(int value)
    {
        return TilemapPropertyValue(TilemapPropertyType::Int, ValueData(value));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateFloat(float value)
    {
        return TilemapPropertyValue(TilemapPropertyType::Float, ValueData(value));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateBool(bool value)
    {
        return TilemapPropertyValue(TilemapPropertyType::Bool, ValueData(value));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateColor(const Color& value)
    {
        return TilemapPropertyValue(TilemapPropertyType::Color, ValueData(value.getPackedValueProperty()));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateFile(std::string path)
    {
        return TilemapPropertyValue(TilemapPropertyType::File, ValueData(), std::move(path));
    }

    TilemapPropertyValue TilemapPropertyValue::CreateObject(int objectId)
    {
        return TilemapPropertyValue(TilemapPropertyType::Object, ValueData(objectId));
    }

    const std::string& TilemapPropertyValue::AsString() const
    {
        if (type_ != TilemapPropertyType::String)
        {
            throw System::InvalidOperationException("Cannot access property as String.");
        }
        return stringValue_;
    }

    int TilemapPropertyValue::AsInt() const
    {
        if (type_ != TilemapPropertyType::Int)
        {
            throw System::InvalidOperationException("Cannot access property as Int.");
        }
        return valueData_.Int32;
    }

    float TilemapPropertyValue::AsFloat() const
    {
        if (type_ != TilemapPropertyType::Float)
        {
            throw System::InvalidOperationException("Cannot access property as Float.");
        }
        return valueData_.Single;
    }

    bool TilemapPropertyValue::AsBool() const
    {
        if (type_ != TilemapPropertyType::Bool)
        {
            throw System::InvalidOperationException("Cannot access property as Bool.");
        }
        return valueData_.Boolean;
    }

    Color TilemapPropertyValue::AsColor() const
    {
        if (type_ != TilemapPropertyType::Color)
        {
            throw System::InvalidOperationException("Cannot access property as Color.");
        }
        Color color(0, 0, 0, 0);
        color.setPackedValueProperty(valueData_.ColorPackedValue);
        return color;
    }

    const std::string& TilemapPropertyValue::AsFile() const
    {
        if (type_ != TilemapPropertyType::File)
        {
            throw System::InvalidOperationException("Cannot access property as File.");
        }
        return stringValue_;
    }

    int TilemapPropertyValue::AsObject() const
    {
        if (type_ != TilemapPropertyType::Object)
        {
            throw System::InvalidOperationException("Cannot access property as Object.");
        }
        return valueData_.Int32;
    }
}
