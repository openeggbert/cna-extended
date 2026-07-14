// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Xml/XmlNodeExtensions.hpp"

#include "System/Boolean.hpp"
#include "System/Byte.hpp"
#include "System/Double.hpp"
#include "System/Int16.hpp"
#include "System/Int32.hpp"
#include "System/IndexOutOfRangeException.hpp"
#include "System/SByte.hpp"
#include "System/Single.hpp"
#include "System/UInt16.hpp"
#include "System/UInt32.hpp"
#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlAttributeCollection.hpp"

#include <sstream>

namespace CNA::Extended::Serialization::Xml
{
    namespace
    {
        bool GetAttributeValue(const System::Xml::XmlNode* node, const std::string& attribute, std::string& value)
        {
            value.clear();
            System::Xml::XmlAttributeCollection* attributes = node->getAttributesProperty();
            System::Xml::XmlAttribute* attr = attributes == nullptr ? nullptr : (*attributes)[attribute];
            if (attr != nullptr)
            {
                value = attr->getValueProperty();
            }
            return !value.empty();
        }
    }

    std::string GetStringAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? value : std::string();
    }

    uint8_t GetByteAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::Byte::Parse(value) : 0;
    }

    uint16_t GetUInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::UInt16::Parse(value) : 0;
    }

    int16_t GetInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::Int16::Parse(value) : 0;
    }

    uint32_t GetUInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::UInt32::Parse(value) : 0;
    }

    int32_t GetInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::Int32::Parse(value) : 0;
    }

    float GetSingleAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::Single::Parse(value) : 0.0f;
    }

    double GetDoubleAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? System::Double::Parse(value) : 0.0;
    }

    bool GetBoolAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) && System::Boolean::Parse(value);
    }

    std::vector<uint8_t> GetByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute, int expectedCount)
    {
        std::vector<uint8_t> result(static_cast<std::size_t>(expectedCount), 0);

        std::string value;
        if (GetAttributeValue(node, attribute, value))
        {
            std::istringstream stream(value);
            std::string token;
            for (int i = 0; i < expectedCount; ++i)
            {
                if (!std::getline(stream, token, ','))
                {
                    throw System::IndexOutOfRangeException("Index was outside the bounds of the array.");
                }
                result[static_cast<std::size_t>(i)] = System::Byte::Parse(token);
            }
        }

        return result;
    }

    std::vector<int8_t> GetSignedByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute, int expectedCount)
    {
        std::vector<int8_t> result(static_cast<std::size_t>(expectedCount), 0);

        std::string value;
        if (GetAttributeValue(node, attribute, value))
        {
            std::istringstream stream(value);
            std::string token;
            for (int i = 0; i < expectedCount; ++i)
            {
                if (!std::getline(stream, token, ','))
                {
                    throw System::IndexOutOfRangeException("Index was outside the bounds of the array.");
                }
                result[static_cast<std::size_t>(i)] = System::SByte::Parse(token);
            }
        }

        return result;
    }
}
