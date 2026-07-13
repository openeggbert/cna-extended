// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Serialization/Json/JsonContentLoader.cs. Upstream implements the
// non-generic `IContentLoader` (a generic method on a non-generic interface); per
// ContentManagerExtensions.hpp's header comment, that has no direct virtual-dispatch C++
// equivalent, so this is a plain class with a template `Load<T>` method, duck-type-compatible with
// `CNA::Extended::Content::Load<T>(ContentManager&, path, loader)`'s generic-loader overload.
#pragma once

#include "CNA/Extended/Content/ContentManagerExtensions.hpp"
#include "CNA/Extended/Serialization/Json/MonoGameJsonSerializerOptionsProvider.hpp"
#include "SharpRuntime/SharpRuntimeHelper.hpp"
#include "System/Text/Json/JsonSerializer.hpp"

#include <memory>
#include <string>
#include <vector>

namespace CNA::Extended::Serialization::Json
{
    using CNA::Extended::Content::ContentManager;

    /** @brief Loads JSON content of type T from a ContentManager-relative path, deserializing via System::Text::Json::JsonSerializer. */
    class JsonContentLoader
    {
    public:
        template <typename T>
        [[nodiscard]] T Load(ContentManager& contentManager, const std::string& path) const
        {
            const std::unique_ptr<System::IO::Stream> stream = CNA::Extended::Content::OpenStream(contentManager, path);
            const System::Text::Json::JsonSerializerOptions options = GetOptions(contentManager, path);

            std::vector<SharpRuntime::bytecs> buffer(static_cast<std::size_t>(stream->getLengthProperty()));
            SharpRuntime::intcs totalRead = 0;
            while (totalRead < stream->getLengthProperty())
            {
                const SharpRuntime::intcs read =
                    stream->Read(buffer.data(), totalRead, stream->getLengthProperty() - totalRead);
                if (read == 0)
                {
                    break;
                }
                totalRead += read;
            }

            const std::string json(buffer.begin(), buffer.begin() + totalRead);
            return System::Text::Json::JsonSerializer::Deserialize<T>(json, options);
        }
    };
}
