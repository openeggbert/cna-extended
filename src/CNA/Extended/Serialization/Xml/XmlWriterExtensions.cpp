// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Xml/XmlWriterExtensions.hpp"

namespace CNA::Extended::Serialization::Xml
{
    void WriteAttributeInt(System::Xml::XmlWriter& writer, const std::string& attributeName, int value)
    {
        writer.WriteAttributeString(attributeName, std::to_string(value));
    }

    void WriteAttributeFloat(System::Xml::XmlWriter& writer, const std::string& attributeName, float value)
    {
        writer.WriteAttributeString(attributeName, std::to_string(value));
    }

    void WriteAttributeBool(System::Xml::XmlWriter& writer, const std::string& attributeName, bool value)
    {
        // Matches C#'s bool.ToString() capitalization ("True"/"False"), used by upstream's $"{value}".
        writer.WriteAttributeString(attributeName, value ? "True" : "False");
    }

    void WriteAttributeRectangle(System::Xml::XmlWriter& writer, const std::string& attributeName, const Rectangle& value)
    {
        writer.WriteAttributeString(attributeName,
            std::to_string(value.X) + "," + std::to_string(value.Y) + "," + std::to_string(value.Width) + "," +
                std::to_string(value.Height));
    }

    void WriteAttributeVector2(System::Xml::XmlWriter& writer, const std::string& attributeName, const Vector2& value)
    {
        writer.WriteAttributeString(attributeName, std::to_string(value.X) + "," + std::to_string(value.Y));
    }

    void WriteAttributeVector3(System::Xml::XmlWriter& writer, const std::string& attributeName, const Vector3& value)
    {
        writer.WriteAttributeString(
            attributeName, std::to_string(value.X) + "," + std::to_string(value.Y) + "," + std::to_string(value.Z));
    }
}
