// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Xml/XmlReaderExtensions.cs. Extension methods ->
// free functions, matching this project's established convention.
//
// Representational note: upstream's `reader.GetAttribute(name)` returns `null` for a missing
// attribute, distinguishable from `""` for a present-but-empty one; `sharp-runtime`'s
// `XmlReader::GetAttribute` returns an empty `std::string` for both (verified by reading its
// implementation). These functions therefore treat an empty attribute value as "missing" -- the
// closest available behavior given that underlying limitation, not a deliberate simplification.
//
// `GetAttributeEnum<T>` is generic over any enum type upstream via `Enum.Parse<T>` (reflection);
// C++ has no enum reflection. Ported as a template that takes an explicit `parse` callable
// (`T(*)(const std::string&)` or equivalent) rather than silently only supporting one enum type --
// this is a genuine, documented representational difference, not a narrowing of upstream's actual
// generality. No enum type in this codebase has a call site for this yet (its one real upstream
// caller, `Particles/ParticleEffectSerializer.cs`, is Phase 8 -- not yet ported), so there is
// nothing to verify this against end-to-end yet; the shape is a direct, deliberate translation of
// upstream's contract, not a guess.
#pragma once

#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/Xml/XmlException.hpp"
#include "System/Xml/XmlReader.hpp"

#include <functional>
#include <string>

namespace CNA::Extended::Serialization::Xml
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Vector3;

    /** @throws System::Xml::XmlException the attribute is missing/empty or not a valid int. */
    [[nodiscard]] int GetAttributeInt(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not a valid int (never throws). */
    [[nodiscard]] int GetAttributeInt(const System::Xml::XmlReader& reader, const std::string& attributeName, int defaultValue);

    /** @throws System::Xml::XmlException the attribute is missing/empty or not a valid float. */
    [[nodiscard]] float GetAttributeFloat(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not a valid float (never throws). */
    [[nodiscard]] float GetAttributeFloat(const System::Xml::XmlReader& reader, const std::string& attributeName, float defaultValue);

    /** @throws System::Xml::XmlException the attribute is missing/empty or not "true"/"false". */
    [[nodiscard]] bool GetAttributeBool(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not "true"/"false" (never throws). */
    [[nodiscard]] bool GetAttributeBool(const System::Xml::XmlReader& reader, const std::string& attributeName, bool defaultValue);

    /** @throws System::Xml::XmlException the attribute is missing/empty, or @p parse throws/returns false. */
    template <typename T>
    T GetAttributeEnum(const System::Xml::XmlReader& reader, const std::string& attributeName, const std::function<bool(const std::string&, T&)>& parse)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            throw System::Xml::XmlException("Required attribute '" + attributeName + "' is missing.");
        }

        T result{};
        if (!parse(value, result))
        {
            throw System::Xml::XmlException("Invalid enum format for attribute '" + attributeName + "'. Got '" + value + "'");
        }
        return result;
    }

    /** @brief Returns @p defaultValue if the attribute is missing/empty or @p parse returns false (never throws). */
    template <typename T>
    T GetAttributeEnum(
        const System::Xml::XmlReader& reader, const std::string& attributeName, T defaultValue,
        const std::function<bool(const std::string&, T&)>& parse)
    {
        const std::string value = reader.GetAttribute(attributeName);
        if (value.empty())
        {
            return defaultValue;
        }

        T result{};
        return parse(value, result) ? result : defaultValue;
    }

    /** @throws System::Xml::XmlException the attribute is missing/empty or not "x,y,width,height". */
    [[nodiscard]] Rectangle GetAttributeRectangle(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not "x,y,width,height" (never throws). */
    [[nodiscard]] Rectangle GetAttributeRectangle(
        const System::Xml::XmlReader& reader, const std::string& attributeName, const Rectangle& defaultValue);

    /** @throws System::Xml::XmlException the attribute is missing/empty or not "x,y". */
    [[nodiscard]] Vector2 GetAttributeVector2(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not "x,y" (never throws). */
    [[nodiscard]] Vector2 GetAttributeVector2(
        const System::Xml::XmlReader& reader, const std::string& attributeName, const Vector2& defaultValue);

    /** @throws System::Xml::XmlException the attribute is missing/empty or not "x,y,z". */
    [[nodiscard]] Vector3 GetAttributeVector3(const System::Xml::XmlReader& reader, const std::string& attributeName);
    /** @brief Returns @p defaultValue if the attribute is missing/empty or not "x,y,z" (never throws). */
    [[nodiscard]] Vector3 GetAttributeVector3(
        const System::Xml::XmlReader& reader, const std::string& attributeName, const Vector3& defaultValue);
}
