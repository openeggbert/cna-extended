// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Xml/XmlNode.Extensions.cs. Extension methods ->
// free functions, matching this project's established convention. Every getter returns a default
// value (0/false/empty string) when the attribute is missing or empty, matching upstream exactly
// (no exception is thrown for a missing attribute; a malformed *present* value still throws via
// the underlying numeric parse, matching `Convert.ToXxx`'s own behavior).
//
// `Content/BitmapFonts/BitmapFontFileReader.cpp` already has a private, narrower copy of a few of
// these (GetStringAttribute/GetByteAttribute/GetUInt16Attribute/GetInt16Attribute) predating this
// module -- its own header comment already flagged this as "kept private since nothing else needs
// it yet... worth a public surface later." That consolidation is a small, separate follow-up (it
// touches a file outside this phase's scope) and is not done here; noting it for the next session.
#pragma once

#include "System/Xml/XmlNode.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace CNA::Extended::Serialization::Xml
{
    [[nodiscard]] std::string GetStringAttribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] uint8_t GetByteAttribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] uint16_t GetUInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] int16_t GetInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] uint32_t GetUInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] int32_t GetInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] float GetSingleAttribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] double GetDoubleAttribute(const System::Xml::XmlNode* node, const std::string& attribute);
    [[nodiscard]] bool GetBoolAttribute(const System::Xml::XmlNode* node, const std::string& attribute);

    /** @brief Reads a comma-delimited attribute of @p expectedCount byte values. Missing/empty attribute yields all-zero. */
    [[nodiscard]] std::vector<uint8_t> GetByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute, int expectedCount);
    /** @brief Reads a comma-delimited attribute of @p expectedCount signed-byte values. Missing/empty attribute yields all-zero. */
    [[nodiscard]] std::vector<int8_t> GetSignedByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute, int expectedCount);
}
