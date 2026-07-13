// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Xml/XmlNodeExtensions.hpp"

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

        template <typename T>
        T ParseInvariant(const std::string& value)
        {
            std::istringstream stream(value);
            stream.imbue(std::locale::classic());
            T result{};
            stream >> result;
            return result;
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
        return GetAttributeValue(node, attribute, value) ? static_cast<uint8_t>(ParseInvariant<unsigned int>(value)) : 0;
    }

    uint16_t GetUInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<uint16_t>(value) : 0;
    }

    int16_t GetInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<int16_t>(value) : 0;
    }

    uint32_t GetUInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<uint32_t>(value) : 0;
    }

    int32_t GetInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<int32_t>(value) : 0;
    }

    float GetSingleAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<float>(value) : 0.0f;
    }

    double GetDoubleAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        return GetAttributeValue(node, attribute, value) ? ParseInvariant<double>(value) : 0.0;
    }

    bool GetBoolAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
    {
        std::string value;
        if (!GetAttributeValue(node, attribute, value))
        {
            return false;
        }
        return value == "true" || value == "True" || value == "1";
    }

    std::vector<uint8_t> GetByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute, int expectedCount)
    {
        std::vector<uint8_t> result(static_cast<std::size_t>(expectedCount), 0);

        std::string value;
        if (GetAttributeValue(node, attribute, value))
        {
            std::istringstream stream(value);
            std::string token;
            for (int i = 0; i < expectedCount && std::getline(stream, token, ','); ++i)
            {
                result[static_cast<std::size_t>(i)] = static_cast<uint8_t>(ParseInvariant<unsigned int>(token));
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
            for (int i = 0; i < expectedCount && std::getline(stream, token, ','); ++i)
            {
                result[static_cast<std::size_t>(i)] = static_cast<int8_t>(ParseInvariant<int>(token));
            }
        }

        return result;
    }
}
