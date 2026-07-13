// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/TextContentLoader.cs. `IContentLoader<string>`
// has a fixed T (string), so unlike JsonContentLoader this maps cleanly onto
// `CNA::Extended::Content::IContentLoader<T>` (see ContentManagerExtensions.hpp).
#pragma once

#include "CNA/Extended/Content/ContentManagerExtensions.hpp"
#include "System/IO/StreamReader.hpp"

#include <memory>
#include <string>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Content::ContentManager;
    using CNA::Extended::Content::IContentLoader;

    /** @brief Loads the full text content of a ContentManager-relative path. */
    class TextContentLoader : public IContentLoader<std::string>
    {
    public:
        [[nodiscard]] std::string Load(ContentManager& contentManager, const std::string& path) override
        {
            const std::unique_ptr<System::IO::Stream> stream = CNA::Extended::Content::OpenStream(contentManager, path);
            System::IO::StreamReader reader(stream.get());
            return reader.ReadToEnd();
        }
    };
}
