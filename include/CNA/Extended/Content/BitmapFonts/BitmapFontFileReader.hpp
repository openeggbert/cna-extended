// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/BitmapFonts/BitmapFontFileReader.cs. A direct parser
// for the AngleCode BMFont `.fnt` spec (binary/text/XML variants) -- no xnb/ContentReader/
// ContentManager dependency anywhere in the upstream file. Upstream's internal static class ->
// free functions in this namespace, matching this project's established convention (see
// Content/ExternalResourceResolvers.hpp, Content/TexturePacker/TexturePackerFileReader.hpp).
#pragma once

#include "CNA/Extended/Content/BitmapFonts/BitmapFontFileContent.hpp"
#include "System/IO/Stream.hpp"

#include <string>

namespace CNA::Extended::Content::BitmapFonts
{
    /**
     * @brief Reads and parses a BMFont file at the given path (auto-detecting binary/text/XML format).
     * @param path The path to the font file to read.
     * @return The parsed file content.
     * @throws System::InvalidOperationException if the file does not appear to be a valid BMFont file.
     */
    [[nodiscard]] BitmapFontFileContent Read(const std::string& path);

    /**
     * @brief Reads and parses BMFont content from an open stream (auto-detecting binary/text/XML format).
     * @param stream A stream containing the font file contents to read.
     * @param name The name or path that uniquely identifies this content.
     * @return The parsed file content.
     * @throws System::InvalidOperationException if the stream does not appear to hold a valid BMFont file.
     */
    [[nodiscard]] BitmapFontFileContent Read(System::IO::Stream& stream, const std::string& name);
}
