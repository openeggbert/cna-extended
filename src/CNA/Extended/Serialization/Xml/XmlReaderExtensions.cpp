// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Uses System::Int32::Parse/System::Single::Parse (not std::stoi/std::stof) to match upstream's
// int.Parse/float.Parse strictness: std::stoi/std::stof only parse a leading numeric prefix and
// silently ignore trailing garbage (e.g. std::stoi("1.5") == 1, no exception), so a malformed
// attribute like "1.5,2,3,4" would silently succeed instead of throwing, unlike upstream's
// int.Parse("1.5") (throws FormatException) or this project's own GetAttributeRectangle tests.
#include "CNA/Extended/Serialization/Xml/XmlReaderExtensions.hpp"

#include "System/Int32.hpp"
#include "System/Single.hpp"

#include <sstream>
#include <vector>

namespace CNA::Extended::Serialization::Xml
{
    namespace
    {
        std::vector<std::string> SplitNonEmpty(const std::string& value, char delimiter)
        {
            std::vector<std::string> result;
            std::string token;
            std::istringstream stream(value);
            while (std::getline(stream, token, delimiter))
            {
                if (!token.empty())
                {
                    result.push_back(token);
                }
            }
            return result;
        }
    }

    int GetAttributeInt(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        try
        {
            return System::Int32::Parse(value);
        }
        catch (const std::exception&)
        {
            throw System::Xml::XmlException(
                "Invalid integer format for attribute '" + attributeName + "'. Expected integer, but got '" + value + "'");
        }
    }

    int GetAttributeInt(const System::Xml::XmlReader& reader, const std::string& attributeName, int defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        try
        {
            return System::Int32::Parse(value);
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }

    float GetAttributeFloat(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        try
        {
            return System::Single::Parse(value);
        }
        catch (const std::exception&)
        {
            throw System::Xml::XmlException(
                "Invalid float format for attribute '" + attributeName + "'. Expected float, but got '" + value + "'");
        }
    }

    float GetAttributeFloat(const System::Xml::XmlReader& reader, const std::string& attributeName, float defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        try
        {
            return System::Single::Parse(value);
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }

    namespace
    {
        bool TryParseBool(const std::string& value, bool& result)
        {
            if (value == "true" || value == "True")
            {
                result = true;
                return true;
            }
            if (value == "false" || value == "False")
            {
                result = false;
                return true;
            }
            return false;
        }
    }

    bool GetAttributeBool(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        bool result = false;
        if (!TryParseBool(value, result))
        {
            throw System::Xml::XmlException(
                "Invalid bool format for attribute '" + attributeName + "'. Expected 'true' or 'false' but got '" + value + "'");
        }
        return result;
    }

    bool GetAttributeBool(const System::Xml::XmlReader& reader, const std::string& attributeName, bool defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        bool result = false;
        return TryParseBool(value, result) ? result : defaultValue;
    }

    Rectangle GetAttributeRectangle(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 4)
        {
            throw System::Xml::XmlException(
                "Invalid Rectangle format for attribute '" + attributeName + "'. Expected 'x,y,width,height' but got '" + value + "'");
        }

        try
        {
            return Rectangle(System::Int32::Parse(split[0]), System::Int32::Parse(split[1]), System::Int32::Parse(split[2]), System::Int32::Parse(split[3]));
        }
        catch (const std::exception&)
        {
            throw System::Xml::XmlException(
                "Invalid Rectangle format for attribute '" + attributeName + "'. Expected 'x,y,width,height' but got '" + value + "'");
        }
    }

    Rectangle GetAttributeRectangle(
        const System::Xml::XmlReader& reader, const std::string& attributeName, const Rectangle& defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 4)
        {
            return defaultValue;
        }

        try
        {
            return Rectangle(System::Int32::Parse(split[0]), System::Int32::Parse(split[1]), System::Int32::Parse(split[2]), System::Int32::Parse(split[3]));
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }

    Vector2 GetAttributeVector2(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 2)
        {
            throw System::Xml::XmlException(
                "Invalid Vector2 format for attribute '" + attributeName + "'. Expected 'x,y', but got '" + value + "'");
        }

        try
        {
            return Vector2(System::Single::Parse(split[0]), System::Single::Parse(split[1]));
        }
        catch (const std::exception&)
        {
            throw System::Xml::XmlException(
                "Invalid Vector2 format for attribute '" + attributeName + "'. Expected 'x,y', but got '" + value + "'");
        }
    }

    Vector2 GetAttributeVector2(const System::Xml::XmlReader& reader, const std::string& attributeName, const Vector2& defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 2)
        {
            return defaultValue;
        }

        try
        {
            return Vector2(System::Single::Parse(split[0]), System::Single::Parse(split[1]));
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }

    Vector3 GetAttributeVector3(const System::Xml::XmlReader& reader, const std::string& attributeName)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 3)
        {
            throw System::Xml::XmlException(
                "Invalid Vector3 format for attribute '" + attributeName + "'. Expected 'x,y,z', but got '" + value + "'");
        }

        try
        {
            return Vector3(System::Single::Parse(split[0]), System::Single::Parse(split[1]), System::Single::Parse(split[2]));
        }
        catch (const std::exception&)
        {
            throw System::Xml::XmlException(
                "Invalid Vector3 format for attribute '" + attributeName + "'. Expected 'x,y,z', but got '" + value + "'");
        }
    }

    Vector3 GetAttributeVector3(const System::Xml::XmlReader& reader, const std::string& attributeName, const Vector3& defaultValue)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        const std::vector<std::string> split = SplitNonEmpty(value, ',');
        if (split.size() != 3)
        {
            return defaultValue;
        }

        try
        {
            return Vector3(System::Single::Parse(split[0]), System::Single::Parse(split[1]), System::Single::Parse(split[2]));
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }
}
