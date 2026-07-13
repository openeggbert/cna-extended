// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Serialization/Xml/XmlReaderExtensionsTests.cs.
// `MoveToContent()` has no equivalent in sharp-runtime's XmlReader; a single `Read()` call
// positions on the root element for these simple single-element documents, matching the pattern
// already established by sharp-runtime's own XmlReaderTests (XmlTests.cpp). The
// `GetAttributeEnum<PlayerIndex>` tests supply an explicit parse callable (see
// XmlReaderExtensions.hpp's header comment for why -- no enum reflection in C++) rather than
// upstream's `Enum.Parse<T>`.
#include "CNA/Extended/Serialization/Xml/XmlReaderExtensions.hpp"

#include "Microsoft/Xna/Framework/PlayerIndex.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Serialization::Xml
{
    using Microsoft::Xna::Framework::PlayerIndex;

    namespace
    {
        std::unique_ptr<System::Xml::XmlReader> CreateXmlReader(const std::string& xml)
        {
            std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
            reader->Read();
            return reader;
        }

        bool TryParsePlayerIndex(const std::string& value, PlayerIndex& result)
        {
            if (value == "One")
            {
                result = PlayerIndex::One;
                return true;
            }
            if (value == "Two")
            {
                result = PlayerIndex::Two;
                return true;
            }
            if (value == "Three")
            {
                result = PlayerIndex::Three;
                return true;
            }
            if (value == "Four")
            {
                result = PlayerIndex::Four;
                return true;
            }
            return false;
        }
    }

    TEST(XmlReaderExtensionsTests, GetAttributeIntValidValueReturnsCorrectInt)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"42\" />");
        EXPECT_EQ(GetAttributeInt(*reader, "testAttr"), 42);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeIntMissingAttributeThrows)
    {
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeInt(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeIntInvalidFormatThrows)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"not-a-number\" />");
        EXPECT_THROW((void)GetAttributeInt(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeFloatValidValueReturnsCorrectFloat)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"3.14\" />");
        EXPECT_NEAR(GetAttributeFloat(*reader, "testAttr"), 3.14f, 1e-5f);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeFloatMissingAttributeThrows)
    {
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeFloat(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeFloatInvalidFormatThrows)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"not-a-float\" />");
        EXPECT_THROW((void)GetAttributeFloat(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeBoolValidValuesReturnsCorrectBool)
    {
        EXPECT_TRUE(GetAttributeBool(*CreateXmlReader("<element testAttr=\"true\" />"), "testAttr"));
        EXPECT_FALSE(GetAttributeBool(*CreateXmlReader("<element testAttr=\"false\" />"), "testAttr"));
        EXPECT_TRUE(GetAttributeBool(*CreateXmlReader("<element testAttr=\"True\" />"), "testAttr"));
        EXPECT_FALSE(GetAttributeBool(*CreateXmlReader("<element testAttr=\"False\" />"), "testAttr"));
    }

    TEST(XmlReaderExtensionsTests, GetAttributeBoolInvalidFormatThrows)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"maybe\" />");
        EXPECT_THROW((void)GetAttributeBool(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeEnumValidValuesReturnsCorrectEnum)
    {
        const std::function<bool(const std::string&, PlayerIndex&)> parse = TryParsePlayerIndex;

        EXPECT_EQ(GetAttributeEnum<PlayerIndex>(*CreateXmlReader("<element testAttr=\"One\" />"), "testAttr", parse), PlayerIndex::One);
        EXPECT_EQ(GetAttributeEnum<PlayerIndex>(*CreateXmlReader("<element testAttr=\"Two\" />"), "testAttr", parse), PlayerIndex::Two);
        EXPECT_EQ(
            GetAttributeEnum<PlayerIndex>(*CreateXmlReader("<element testAttr=\"Three\" />"), "testAttr", parse), PlayerIndex::Three);
        EXPECT_EQ(GetAttributeEnum<PlayerIndex>(*CreateXmlReader("<element testAttr=\"Four\" />"), "testAttr", parse), PlayerIndex::Four);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeEnumMissingAttributeThrows)
    {
        const std::function<bool(const std::string&, PlayerIndex&)> parse = TryParsePlayerIndex;
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeEnum<PlayerIndex>(*reader, "testAttr", parse), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeEnumInvalidValueThrows)
    {
        const std::function<bool(const std::string&, PlayerIndex&)> parse = TryParsePlayerIndex;
        const auto reader = CreateXmlReader("<element testAttr=\"InvalidEnumValue\" />");
        EXPECT_THROW((void)GetAttributeEnum<PlayerIndex>(*reader, "testAttr", parse), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeRectangleValidValueReturnsCorrectRectangle)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"1,2,3,4\" />");
        const Rectangle actual = GetAttributeRectangle(*reader, "testAttr");
        EXPECT_EQ(actual, Rectangle(1, 2, 3, 4));
    }

    TEST(XmlReaderExtensionsTests, GetAttributeRectangleMissingAttributeThrows)
    {
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeRectangle(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeRectangleInvalidFormatThrows)
    {
        for (const std::string value : {"1,2,3", "1,2,3,4,5", "a,b,c,d", "1.5,2,3,4"})
        {
            const auto reader = CreateXmlReader("<element testAttr=\"" + value + "\" />");
            EXPECT_THROW((void)GetAttributeRectangle(*reader, "testAttr"), System::Xml::XmlException);
        }
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector2ValidValueReturnsCorrectVector2)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"1,2\" />");
        const Vector2 actual = GetAttributeVector2(*reader, "testAttr");
        EXPECT_FLOAT_EQ(actual.X, 1.0f);
        EXPECT_FLOAT_EQ(actual.Y, 2.0f);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector2MissingAttributeThrows)
    {
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeVector2(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector2InvalidFormatThrows)
    {
        for (const std::string value : {"3.14", "1,2,3", "a,b"})
        {
            const auto reader = CreateXmlReader("<element testAttr=\"" + value + "\" />");
            EXPECT_THROW((void)GetAttributeVector2(*reader, "testAttr"), System::Xml::XmlException);
        }
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector3ValidValueReturnsCorrectVector3)
    {
        const auto reader = CreateXmlReader("<element testAttr=\"1,2,3\" />");
        const Vector3 actual = GetAttributeVector3(*reader, "testAttr");
        EXPECT_FLOAT_EQ(actual.X, 1.0f);
        EXPECT_FLOAT_EQ(actual.Y, 2.0f);
        EXPECT_FLOAT_EQ(actual.Z, 3.0f);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector3MissingAttributeThrows)
    {
        const auto reader = CreateXmlReader("<element />");
        EXPECT_THROW((void)GetAttributeVector3(*reader, "testAttr"), System::Xml::XmlException);
    }

    TEST(XmlReaderExtensionsTests, GetAttributeVector3InvalidFormatThrows)
    {
        for (const std::string value : {"1.0,2.0", "1,2,3,4", "a,b,c"})
        {
            const auto reader = CreateXmlReader("<element testAttr=\"" + value + "\" />");
            EXPECT_THROW((void)GetAttributeVector3(*reader, "testAttr"), System::Xml::XmlException);
        }
    }
}
