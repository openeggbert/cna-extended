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

    // Regression tests: a malformed *present* value must throw (matching upstream's
    // Convert.ToXxx(string, CultureInfo.InvariantCulture) behavior), not silently return a
    // default/truncated value -- this header's own top comment already documented this contract,
    // but the .cpp didn't actually implement it (ParseInvariant<T> used unchecked istringstream
    // extraction). Found via a member-level audit against upstream, fixed by switching to
    // sharp-runtime's System::Byte::Parse/System::UInt16::Parse/etc (already the established
    // pattern in the sibling XmlReaderExtensions.cpp).
    TEST(XmlNodeExtensionsTests, GetByteAttributeMalformedThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="not-a-number" />)");
        EXPECT_THROW((void)GetByteAttribute(root, "value"), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetByteAttributeOutOfRangeThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="300" />)");
        EXPECT_THROW((void)GetByteAttribute(root, "value"), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetInt32AttributeMalformedThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="12abc" />)");
        EXPECT_THROW((void)GetInt32Attribute(root, "value"), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetSingleAttributeMalformedThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="not-a-float" />)");
        EXPECT_THROW((void)GetSingleAttribute(root, "value"), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetBoolAttributeMalformedThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="yes" />)");
        EXPECT_THROW((void)GetBoolAttribute(root, "value"), std::exception);
    }

    // "1" is a plausible-looking boolean in many formats, but Convert.ToBoolean(string) only
    // recognizes "True"/"False" (case-insensitive) -- confirm the stricter behavior is preserved,
    // not silently widened to accept "1"/"0" as some naive implementations do.
    TEST(XmlNodeExtensionsTests, GetBoolAttributeRejectsNumericOne)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="1" />)");
        EXPECT_THROW((void)GetBoolAttribute(root, "value"), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetByteDelimitedAttributeTooFewTokensThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="1,2" />)");
        EXPECT_THROW((void)GetByteDelimitedAttribute(root, "value", 4), std::exception);
    }

    TEST(XmlNodeExtensionsTests, GetSignedByteDelimitedAttributeTooFewTokensThrows)
    {
        System::Xml::XmlDocument document;
        System::Xml::XmlElement* root = ParseRoot(document, R"(<element value="-1" />)");
        EXPECT_THROW((void)GetSignedByteDelimitedAttribute(root, "value", 3), std::exception);
    }
}
