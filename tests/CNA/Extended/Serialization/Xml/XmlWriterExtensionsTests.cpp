// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Serialization/Xml/XmlWriterExtensionsTests.cs.
// Upstream asserts exact string equality against .NET's XmlWriter output (e.g. `<test value="1" />`).
// This port's XmlWriter is backed by tinyxml2 (via sharp-runtime), whose exact self-closing-tag/
// whitespace formatting is not guaranteed to match .NET's byte-for-byte -- sharp-runtime's own
// XmlWriter test suite (XmlTests.cpp) already established substring `.find()` checks instead of
// exact equality for exactly this reason. Following that precedent here rather than asserting a
// specific formatting this project doesn't control.
#include "CNA/Extended/Serialization/Xml/XmlWriterExtensions.hpp"

#include "System/Xml/XmlWriterSettings.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace CNA::Extended::Serialization::Xml
{
    namespace
    {
        std::unique_ptr<System::Xml::XmlWriter> CreateXmlWriter()
        {
            System::Xml::XmlWriterSettings settings;
            settings.OmitXmlDeclaration = true;
            settings.Indent = true;
            return std::unique_ptr<System::Xml::XmlWriter>(System::Xml::XmlWriter::CreateToString(settings));
        }
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeIntWritesCorrectAttribute)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();

        writer->WriteStartElement("test");
        WriteAttributeInt(*writer, "value", 1);
        writer->WriteEndElement();

        EXPECT_NE(writer->ToString().find("value=\"1\""), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeFloatWritesCorrectAttribute)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();

        writer->WriteStartElement("test");
        WriteAttributeFloat(*writer, "value", 1.2f);
        writer->WriteEndElement();

        EXPECT_NE(writer->ToString().find("value=\"1.2"), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeBoolWritesTrue)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();

        writer->WriteStartElement("test");
        WriteAttributeBool(*writer, "value", true);
        writer->WriteEndElement();

        EXPECT_NE(writer->ToString().find("value=\"True\""), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeBoolWritesFalse)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();

        writer->WriteStartElement("test");
        WriteAttributeBool(*writer, "value", false);
        writer->WriteEndElement();

        EXPECT_NE(writer->ToString().find("value=\"False\""), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeRectangleWritesCorrectAttribute)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();
        const Rectangle rectangle(1, 2, 3, 4);

        writer->WriteStartElement("test");
        WriteAttributeRectangle(*writer, "value", rectangle);
        writer->WriteEndElement();

        EXPECT_NE(writer->ToString().find("value=\"1,2,3,4\""), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeVector2WritesCorrectValue)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();
        const Vector2 vector(1.1f, 2.2f);

        writer->WriteStartElement("test");
        WriteAttributeVector2(*writer, "value", vector);
        writer->WriteEndElement();

        const std::string out = writer->ToString();
        EXPECT_NE(out.find("value=\"1.1"), std::string::npos);
        EXPECT_NE(out.find("2.2"), std::string::npos);
    }

    TEST(XmlWriterExtensionsTests, WriteAttributeVector3WritesCorrectValue)
    {
        const std::unique_ptr<System::Xml::XmlWriter> writer = CreateXmlWriter();
        const Vector3 vector(1.1f, 2.2f, 3.3f);

        writer->WriteStartElement("test");
        WriteAttributeVector3(*writer, "value", vector);
        writer->WriteEndElement();

        const std::string out = writer->ToString();
        EXPECT_NE(out.find("value=\"1.1"), std::string::npos);
        EXPECT_NE(out.find("2.2"), std::string::npos);
        EXPECT_NE(out.find("3.3"), std::string::npos);
    }
}
