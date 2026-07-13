// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Serialization/Xml/XmlWriterExtensions.hpp"

#include "System/Single.hpp"

namespace CNA::Extended::Serialization::Xml
{
    void WriteAttributeInt(System::Xml::XmlWriter& writer, const std::string& attributeName, int value)
    {
        writer.WriteAttributeString(attributeName, std::to_string(value));
    }

    void WriteAttributeFloat(System::Xml::XmlWriter& writer, const std::string& attributeName, float value)
    {
        // std::to_string(float) always emits fixed 6-decimal notation ("60.000000"), unlike C#'s
        // $"{value}" (float.ToString()), which emits the shortest round-trippable representation
        // ("60"). System::Single::ToString uses std::to_chars for that same shortest-round-trip
        // behavior, matching upstream's XmlWriterExtensions.cs exactly.
        writer.WriteAttributeString(attributeName, System::Single::ToString(value));
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
        writer.WriteAttributeString(attributeName, System::Single::ToString(value.X) + "," + System::Single::ToString(value.Y));
    }

    void WriteAttributeVector3(System::Xml::XmlWriter& writer, const std::string& attributeName, const Vector3& value)
    {
        writer.WriteAttributeString(attributeName,
            System::Single::ToString(value.X) + "," + System::Single::ToString(value.Y) + "," + System::Single::ToString(value.Z));
    }
}
