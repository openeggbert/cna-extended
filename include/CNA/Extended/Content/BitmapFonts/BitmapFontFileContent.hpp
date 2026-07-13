// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Content/BitmapFonts/BitmapFontFileContent.cs. Upstream's block
// structs use `[StructLayout(LayoutKind.Explicit)]`/`[FieldOffset(n)]` so `Marshal.PtrToStructure`
// can reinterpret a raw byte buffer directly onto the struct -- a .NET-marshaling-specific
// mechanism with no safe, portable C++ equivalent (relying on C++ struct layout/padding to match
// exact byte offsets across compilers would be fragile, not faithful). These block types are kept
// as plain structs with the same named fields in the same order; `BitmapFontFileReader.cpp`
// reproduces the exact same field values by reading each field individually at its point of use
// (via `BinaryReader` for the binary format, node attributes for XML, tokens for text) rather than
// one raw reinterpret -- same end result (identical field values), different, portable mechanism.
// Upstream's plain public fields (not C# properties) are kept as plain public fields here too,
// matching this project's established `Size`/`RectangleF`-style convention for such types.
#pragma once

#include "SharpRuntime/SharpRuntimeHelper.hpp"

#include <string>
#include <vector>

namespace CNA::Extended::Content::BitmapFonts
{
    using SharpRuntime::bytecs;
    using SharpRuntime::intcs;
    using SharpRuntime::sbytecs;
    using SharpRuntime::shortcs;
    using SharpRuntime::uintcs;
    using SharpRuntime::ushortcs;

    /** @brief The 4-byte BMFont binary format header. */
    struct HeaderBlock
    {
        static constexpr intcs StructSize = 4;

        bytecs B = 0;
        bytecs M = 0;
        bytecs F = 0;
        bytecs Version = 0;

        [[nodiscard]] bool getIsValidProperty() const { return B == 0x42 && M == 0x4D && F == 0x46 && Version == 3; }
    };

    /** @brief The BMFont "info" block. */
    struct InfoBlock
    {
        static constexpr intcs StructSize = 14;

        shortcs FontSize = 0;
        bytecs BitField = 0;
        bytecs CharSet = 0;
        ushortcs StretchH = 0;
        bytecs AA = 0;
        bytecs PaddingUp = 0;
        bytecs PaddingRight = 0;
        bytecs PaddingDown = 0;
        bytecs PaddingLeft = 0;
        sbytecs SpacingHoriz = 0;
        sbytecs SpacingVert = 0;
        bytecs Outline = 0;
    };

    /** @brief One "char" entry of the BMFont format. */
    struct CharacterBlock
    {
        static constexpr intcs StructSize = 20;

        intcs ID = 0;
        ushortcs X = 0;
        ushortcs Y = 0;
        ushortcs Width = 0;
        ushortcs Height = 0;
        shortcs XOffset = 0;
        shortcs YOffset = 0;
        shortcs XAdvance = 0;
        bytecs Page = 0;
        bytecs Chnl = 0;
    };

    /** @brief The BMFont "common" block. */
    struct CommonBlock
    {
        static constexpr intcs StructSize = 15;

        ushortcs LineHeight = 0;
        ushortcs Base = 0;
        ushortcs ScaleW = 0;
        ushortcs ScaleH = 0;
        ushortcs Pages = 0;
        bytecs BitField = 0;
        bytecs AlphaChnl = 0;
        bytecs RedChnl = 0;
        bytecs GreenChnl = 0;
        bytecs BlueChnl = 0;
    };

    /** @brief One "kerning" entry of the BMFont format. */
    struct KerningPairsBlock
    {
        static constexpr intcs StructSize = 10;

        uintcs First = 0;
        uintcs Second = 0;
        shortcs Amount = 0;
    };

    /** @brief The fully-parsed contents of a BMFont `.fnt` file (binary, text, or XML variant). */
    class BitmapFontFileContent
    {
    public:
        std::string Path;
        HeaderBlock Header;
        CommonBlock Common;
        InfoBlock Info;
        std::string FontName;
        std::vector<std::string> Pages;
        std::vector<CharacterBlock> Characters;
        std::vector<KerningPairsBlock> Kernings;
    };
}
