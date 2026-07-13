// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/BitmapFonts/BitmapFontFileReaderTests.cs.
// Fixture .fnt files copied verbatim from upstream's tests/MonoGame.Extended.Tests/BitmapFonts/files/bmfont/.
// Upstream resolves fixture paths relative to the test runner's working directory; this project's
// test binary's working directory isn't pinned to the source tree by CMakeLists.txt (no existing
// precedent for source-relative test fixtures anywhere in this project or in cna/sharp-runtime's
// own test suites), so fixture paths are resolved relative to this .cpp file's own location via
// __FILE__ instead -- correct regardless of the CWD the test binary happens to be invoked from.
#include "CNA/Extended/Content/BitmapFonts/BitmapFontFileReader.hpp"

#include "System/IO/FileStream.hpp"

#include <filesystem>
#include <gtest/gtest.h>

namespace CNA::Extended::Content::BitmapFonts
{
    namespace
    {
        std::filesystem::path FixturePath(const std::string& fileName)
        {
            return std::filesystem::path(__FILE__).parent_path() / "files" / "bmfont" / fileName;
        }

        BitmapFontFileContent CreateExpected()
        {
            BitmapFontFileContent bmfFile;
            bmfFile.Header.B = static_cast<SharpRuntime::bytecs>('B');
            bmfFile.Header.M = static_cast<SharpRuntime::bytecs>('M');
            bmfFile.Header.F = static_cast<SharpRuntime::bytecs>('F');
            bmfFile.Header.Version = 3;

            bmfFile.Info.FontSize = 32;
            bmfFile.Info.BitField = 0b1100'0000;
            bmfFile.Info.CharSet = 0;
            bmfFile.Info.StretchH = 50;
            bmfFile.Info.AA = 1;
            bmfFile.Info.PaddingUp = 1;
            bmfFile.Info.PaddingRight = 2;
            bmfFile.Info.PaddingDown = 3;
            bmfFile.Info.PaddingLeft = 4;
            bmfFile.Info.SpacingHoriz = 6;
            bmfFile.Info.SpacingVert = 5;
            bmfFile.Info.Outline = 2;

            bmfFile.FontName = "Cute Dino";

            bmfFile.Common.LineHeight = 16;
            bmfFile.Common.Base = 12;
            bmfFile.Common.ScaleW = 256;
            bmfFile.Common.ScaleH = 256;
            bmfFile.Common.Pages = 1;
            bmfFile.Common.BitField = 0b0000'0000;
            bmfFile.Common.AlphaChnl = 1;
            bmfFile.Common.RedChnl = 0;
            bmfFile.Common.GreenChnl = 0;
            bmfFile.Common.BlueChnl = 0;

            bmfFile.Pages.push_back("test-font_0.png");

            CharacterBlock charF;
            charF.ID = 70;
            charF.X = 34;
            charF.Y = 0;
            charF.Width = 27;
            charF.Height = 20;
            charF.XOffset = -5;
            charF.YOffset = -3;
            charF.XAdvance = 17;
            charF.Page = 0;
            charF.Chnl = 15;
            bmfFile.Characters.push_back(charF);

            CharacterBlock charJ;
            charJ.ID = 74;
            charJ.X = 0;
            charJ.Y = 0;
            charJ.Width = 28;
            charJ.Height = 20;
            charJ.XOffset = -6;
            charJ.YOffset = -3;
            charJ.XAdvance = 18;
            charJ.Page = 0;
            charJ.Chnl = 15;
            bmfFile.Characters.push_back(charJ);

            KerningPairsBlock kerning;
            kerning.First = 70;
            kerning.Second = 74;
            kerning.Amount = -1;
            bmfFile.Kernings.push_back(kerning);

            return bmfFile;
        }

        void ExpectMatches(const BitmapFontFileContent& expected, const BitmapFontFileContent& actual)
        {
            EXPECT_EQ(expected.Header.B, actual.Header.B);
            EXPECT_EQ(expected.Header.M, actual.Header.M);
            EXPECT_EQ(expected.Header.F, actual.Header.F);
            EXPECT_EQ(expected.Header.Version, actual.Header.Version);

            EXPECT_EQ(expected.Info.FontSize, actual.Info.FontSize);
            EXPECT_EQ(expected.Info.BitField, actual.Info.BitField);
            EXPECT_EQ(expected.Info.CharSet, actual.Info.CharSet);
            EXPECT_EQ(expected.Info.StretchH, actual.Info.StretchH);
            EXPECT_EQ(expected.Info.AA, actual.Info.AA);
            EXPECT_EQ(expected.Info.PaddingUp, actual.Info.PaddingUp);
            EXPECT_EQ(expected.Info.PaddingRight, actual.Info.PaddingRight);
            EXPECT_EQ(expected.Info.PaddingDown, actual.Info.PaddingDown);
            EXPECT_EQ(expected.Info.PaddingLeft, actual.Info.PaddingLeft);
            EXPECT_EQ(expected.Info.SpacingHoriz, actual.Info.SpacingHoriz);
            EXPECT_EQ(expected.Info.SpacingVert, actual.Info.SpacingVert);
            EXPECT_EQ(expected.Info.Outline, actual.Info.Outline);

            EXPECT_EQ(expected.Common.LineHeight, actual.Common.LineHeight);
            EXPECT_EQ(expected.Common.Base, actual.Common.Base);
            EXPECT_EQ(expected.Common.ScaleW, actual.Common.ScaleW);
            EXPECT_EQ(expected.Common.ScaleH, actual.Common.ScaleH);
            EXPECT_EQ(expected.Common.Pages, actual.Common.Pages);
            EXPECT_EQ(expected.Common.BitField, actual.Common.BitField);
            EXPECT_EQ(expected.Common.AlphaChnl, actual.Common.AlphaChnl);
            EXPECT_EQ(expected.Common.RedChnl, actual.Common.RedChnl);
            EXPECT_EQ(expected.Common.GreenChnl, actual.Common.GreenChnl);
            EXPECT_EQ(expected.Common.BlueChnl, actual.Common.BlueChnl);

            EXPECT_EQ(expected.FontName, actual.FontName);
            ASSERT_EQ(expected.Pages, actual.Pages);

            ASSERT_EQ(expected.Characters.size(), actual.Characters.size());
            for (std::size_t i = 0; i < expected.Characters.size(); ++i)
            {
                EXPECT_EQ(expected.Characters[i].ID, actual.Characters[i].ID);
                EXPECT_EQ(expected.Characters[i].X, actual.Characters[i].X);
                EXPECT_EQ(expected.Characters[i].Y, actual.Characters[i].Y);
                EXPECT_EQ(expected.Characters[i].Width, actual.Characters[i].Width);
                EXPECT_EQ(expected.Characters[i].Height, actual.Characters[i].Height);
                EXPECT_EQ(expected.Characters[i].XOffset, actual.Characters[i].XOffset);
                EXPECT_EQ(expected.Characters[i].YOffset, actual.Characters[i].YOffset);
                EXPECT_EQ(expected.Characters[i].XAdvance, actual.Characters[i].XAdvance);
                EXPECT_EQ(expected.Characters[i].Page, actual.Characters[i].Page);
                EXPECT_EQ(expected.Characters[i].Chnl, actual.Characters[i].Chnl);
            }

            ASSERT_EQ(expected.Kernings.size(), actual.Kernings.size());
            for (std::size_t i = 0; i < expected.Kernings.size(); ++i)
            {
                EXPECT_EQ(expected.Kernings[i].First, actual.Kernings[i].First);
                EXPECT_EQ(expected.Kernings[i].Second, actual.Kernings[i].Second);
                EXPECT_EQ(expected.Kernings[i].Amount, actual.Kernings[i].Amount);
            }
        }
    }

    TEST(BitmapFontFileReaderTests, ReadBinaryFile)
    {
        const std::filesystem::path path = FixturePath("test-font-binary.fnt");
        System::IO::FileStream stream(path.string());

        const BitmapFontFileContent actual = Read(stream, path.string());

        ExpectMatches(CreateExpected(), actual);
    }

    TEST(BitmapFontFileReaderTests, ReadXmlFile)
    {
        const std::filesystem::path path = FixturePath("test-font-xml.fnt");
        System::IO::FileStream stream(path.string());

        const BitmapFontFileContent actual = Read(stream, path.string());

        ExpectMatches(CreateExpected(), actual);
    }

    // Issue: MonoGame.Extended won't load XML format .fnt files if they begin with the byte order
    // mark. https://github.com/MonoGame-Extended/Monogame-Extended/issues/1073
    TEST(BitmapFontFileReaderTests, ReadXmlFileWithUtf8Bom)
    {
        const std::filesystem::path path = FixturePath("test-font-xml-utf8-bom.fnt");
        System::IO::FileStream stream(path.string());

        const BitmapFontFileContent actual = Read(stream, path.string());

        ExpectMatches(CreateExpected(), actual);
    }

    TEST(BitmapFontFileReaderTests, ReadTextFile)
    {
        const std::filesystem::path path = FixturePath("test-font-text.fnt");
        System::IO::FileStream stream(path.string());

        const BitmapFontFileContent actual = Read(stream, path.string());

        ExpectMatches(CreateExpected(), actual);
    }
}
