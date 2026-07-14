// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Particles/ParticleEffectSerializer.cs. Upstream's static class
// with `Deserialize`/`Serialize` methods -> free functions in a matching namespace, per this
// project's established convention for stateless static-method-only C# classes.
//
// `Deserialize` returns `std::unique_ptr<ParticleEffect>`, not `ParticleEffect` by value:
// `ParticleEffect` has a user-declared destructor and a deleted copy constructor (see
// `ParticleEffect.hpp`), which suppresses implicit move-constructor generation too -- it is
// neither copyable nor movable, so a factory function returning a fresh instance must return by
// pointer. Matches `WorldBuilder::Build()`'s identical precedent from the just-landed ECS/Tilemaps
// work this session.
//
// **Real representational gap, worth reading before touching this file**: upstream's read side is
// built entirely on `System.Xml.XmlReader`'s streaming-cursor API -- `ReadToDescendant(name)`,
// `ReadToNextSibling(name)`, and (heavily) `ReadSubtree()` to scope a nested reader to just one
// element's children. `sharp-runtime`'s `System::Xml::XmlReader` is a much more minimal
// DOM-cursor-over-tinyxml2 wrapper that has none of those three methods (only `Read()` +
// node-type/name/attribute access) -- confirmed by reading its header directly, not assumed. The
// read side of this port is therefore built on `System::Xml::XmlDocument`/`XmlElement`/`XmlNode`
// (the DOM API) instead, using `SelectSingleNode`/`SelectNodes` (XPath) for the same
// descend-to-child/iterate-siblings operations upstream's reader methods performed --this is the
// same DOM-based approach `Content/BitmapFonts/BitmapFontFileReader.cpp` already established for
// an equivalent "no XmlReader.ReadSubtree available" problem earlier this session. The write side
// needed no such change: `XmlWriter`'s sequential `WriteStartElement`/`WriteAttributeString`/
// `WriteEndElement` API has no streaming-vs-DOM gap (writing is inherently sequential), so it
// ports directly using the already-existing `Serialization::Xml::XmlWriterExtensions` helpers.
#pragma once

#include "CNA/Extended/Particles/ParticleEffect.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "System/IO/Stream.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Particles::ParticleEffectSerializer
{
    using Microsoft::Xna::Framework::Content::ContentManager;

    /**
     * @brief Deserializes a ParticleEffect from an XML file.
     * @throws System::ArgumentException fileName is empty.
     * @throws System::Xml::XmlException the XML format is invalid.
     */
    [[nodiscard]] std::unique_ptr<ParticleEffect> Deserialize(const std::string& fileName, ContentManager& content);

    /**
     * @brief Deserializes a ParticleEffect from a stream containing XML data.
     * @param stream The stream to read the XML data from.
     * @param content The content manager used to load referenced textures.
     * @param baseDirectory Base directory for resolving relative texture paths; if empty, uses @p content's RootDirectory.
     * @throws System::Xml::XmlException the XML format is invalid.
     */
    [[nodiscard]] std::unique_ptr<ParticleEffect> Deserialize(
        System::IO::Stream& stream, ContentManager& content, const std::string& baseDirectory = "");

    /**
     * @brief Serializes a ParticleEffect to an XML file.
     * @throws System::ArgumentException fileName is empty.
     */
    void Serialize(const std::string& fileName, ParticleEffect& effect);

    /** @brief Serializes a ParticleEffect to a stream as XML data. */
    void Serialize(System::IO::Stream& stream, ParticleEffect& effect);
}
