// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Xml/XmlWriterExtensions.cs. Extension methods ->
// free functions, matching this project's established convention.
#pragma once

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Xml/XmlWriter.hpp"

#include <string>

namespace CNA::Extended::Serialization::Xml
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;

    void WriteAttributeInt(System::Xml::XmlWriter& writer, const std::string& attributeName, int value);
    void WriteAttributeFloat(System::Xml::XmlWriter& writer, const std::string& attributeName, float value);
    void WriteAttributeBool(System::Xml::XmlWriter& writer, const std::string& attributeName, bool value);
    void WriteAttributeRectangle(System::Xml::XmlWriter& writer, const std::string& attributeName, const Rectangle& value);
    void WriteAttributeVector2(System::Xml::XmlWriter& writer, const std::string& attributeName, const Vector2& value);
    void WriteAttributeVector3(System::Xml::XmlWriter& writer, const std::string& attributeName, const Vector3& value);
}
