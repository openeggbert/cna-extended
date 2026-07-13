// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/TexturePacker/TexturePackerFileReader.cs. Upstream's
// internal static class -> free functions in this namespace, matching this project's established
// convention for stateless static-method-only C# classes (see
// Content/ExternalResourceResolvers.hpp). Upstream's `internal` visibility is kept public here,
// matching this project's established internal-visibility convention (see plan.md).
#pragma once

#include "CNA/Extended/Content/TexturePacker/TexturePackerFileContent.hpp"
#include "System/IO/Stream.hpp"

#include <string>

namespace CNA::Extended::Content::TexturePacker
{
    /**
     * @brief Reads and parses a TexturePacker JSON export file from the local file system.
     * @param path The path to the TexturePacker JSON file.
     * @return The parsed file content.
     */
    [[nodiscard]] TexturePackerFileContent Read(const std::string& path);

    /**
     * @brief Reads and parses a TexturePacker JSON export file from an open stream.
     * @param stream The stream to read the TexturePacker JSON content from.
     * @return The parsed file content.
     */
    [[nodiscard]] TexturePackerFileContent Read(System::IO::Stream& stream);
}
