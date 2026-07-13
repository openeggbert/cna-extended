// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for XmlNode.Extensions.cs. Fresh tests below.
#include "CNA/Extended/Serialization/Xml/XmlNodeExtensions.hpp"

#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Serialization::Xml
{
    namespace
    {
        System::Xml::XmlElement* ParseRoot(System::Xml::XmlDocument& document, const std::string& xml)
        {
            document.LoadXml(xml);
            return document.getDocumentElementProperty();
        }
    }

    TEST(XmlNodeExtensionsTests, GetStringAttributeReturnsValue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element name="hello" />)");
        EXPECT_EQ(GetStringAttribute(root, "name"), "hello");
    }

    TEST(XmlNodeExtensionsTests, GetStringAttributeMissingReturnsEmpty)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element />)");
        EXPECT_EQ(GetStringAttribute(root, "name"), "");
    }

    TEST(XmlNodeExtensionsTests, GetByteAttributeParsesValue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="200" />)");
        EXPECT_EQ(GetByteAttribute(root, "value"), 200);
    }

    TEST(XmlNodeExtensionsTests, GetUInt16AttributeParsesValue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="60000" />)");
        EXPECT_EQ(GetUInt16Attribute(root, "value"), 60000);
    }

    TEST(XmlNodeExtensionsTests, GetInt16AttributeParsesValue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="-30000" />)");
        EXPECT_EQ(GetInt16Attribute(root, "value"), -30000);
    }

    TEST(XmlNodeExtensionsTests, GetInt32AttributeMissingReturnsZero)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element />)");
        EXPECT_EQ(GetInt32Attribute(root, "value"), 0);
    }

    TEST(XmlNodeExtensionsTests, GetSingleAttributeParsesValue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="3.5" />)");
        EXPECT_FLOAT_EQ(GetSingleAttribute(root, "value"), 3.5f);
    }

    TEST(XmlNodeExtensionsTests, GetBoolAttributeParsesTrue)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="true" />)");
        EXPECT_TRUE(GetBoolAttribute(root, "value"));
    }

    TEST(XmlNodeExtensionsTests, GetBoolAttributeMissingReturnsFalse)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element />)");
        EXPECT_FALSE(GetBoolAttribute(root, "value"));
    }

    TEST(XmlNodeExtensionsTests, GetByteDelimitedAttributeParsesValues)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="1,2,3,4" />)");
        const std::vector<uint8_t> actual = GetByteDelimitedAttribute(root, "value", 4);
        ASSERT_EQ(actual.size(), 4u);
        EXPECT_EQ(actual[0], 1);
        EXPECT_EQ(actual[1], 2);
        EXPECT_EQ(actual[2], 3);
        EXPECT_EQ(actual[3], 4);
    }

    TEST(XmlNodeExtensionsTests, GetByteDelimitedAttributeMissingReturnsAllZero)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element />)");
        const std::vector<uint8_t> actual = GetByteDelimitedAttribute(root, "value", 3);
        ASSERT_EQ(actual.size(), 3u);
        EXPECT_EQ(actual[0], 0);
        EXPECT_EQ(actual[1], 0);
        EXPECT_EQ(actual[2], 0);
    }

    TEST(XmlNodeExtensionsTests, GetSignedByteDelimitedAttributeParsesValues)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="-1,2,-3" />)");
        const std::vector<int8_t> actual = GetSignedByteDelimitedAttribute(root, "value", 3);
        ASSERT_EQ(actual.size(), 3u);
        EXPECT_EQ(actual[0], -1);
        EXPECT_EQ(actual[1], 2);
        EXPECT_EQ(actual[2], -3);
    }
}
