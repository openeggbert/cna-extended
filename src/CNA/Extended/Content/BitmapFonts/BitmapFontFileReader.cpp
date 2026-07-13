// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Content/BitmapFonts/BitmapFontFileReader.hpp"

#include "System/Convert.hpp"
#include "System/IO/BinaryReader.hpp"
#include "System/IO/File.hpp"
#include "System/IO/FileStream.hpp"
#include "System/IO/StreamReader.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Text/Encoding.hpp"
#include "System/Xml/XmlAttribute.hpp"
#include "System/Xml/XmlAttributeCollection.hpp"
#include "System/Xml/XmlDocument.hpp"
#include "System/Xml/XmlElement.hpp"
#include "System/Xml/XmlNodeList.hpp"

#include <algorithm>
#include <memory>
#include <vector>

namespace CNA::Extended::Content::BitmapFonts
{
    namespace
    {
        // --- XmlNode attribute helpers ---------------------------------------------------------
        // Ported from MonoGame.Extended's Serialization/Xml/XmlNode.Extensions.cs. Kept private to
        // this translation unit (only BitmapFontFileReader's XML variant needs them) rather than
        // landed as a public Serialization/Xml module -- Phase 6 (Serialization) is where a public
        // XmlNode-extensions surface would belong if a second consumer ever needs one.

        bool TryGetAttributeValue(const System::Xml::XmlNode* node, const std::string& attribute, std::string& value)
        {
            value.clear();
            if (const System::Xml::XmlAttributeCollection* attrs = node->getAttributesProperty())
            {
                if (const System::Xml::XmlAttribute* attr = (*attrs)[attribute])
                {
                    value = attr->getValueProperty();
                }
            }
            return !value.empty();
        }

        std::string GetStringAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? value : std::string();
        }

        SharpRuntime::bytecs GetByteAttribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? System::Convert::ToByte(value) : SharpRuntime::bytecs(0);
        }

        SharpRuntime::ushortcs GetUInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? System::Convert::ToUInt16(value) : SharpRuntime::ushortcs(0);
        }

        SharpRuntime::shortcs GetInt16Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? System::Convert::ToInt16(value) : SharpRuntime::shortcs(0);
        }

        SharpRuntime::uintcs GetUInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? System::Convert::ToUInt32(value) : SharpRuntime::uintcs(0);
        }

        SharpRuntime::intcs GetInt32Attribute(const System::Xml::XmlNode* node, const std::string& attribute)
        {
            std::string value;
            return TryGetAttributeValue(node, attribute, value) ? System::Convert::ToInt32(value) : SharpRuntime::intcs(0);
        }

        std::vector<std::string> SplitByComma(const std::string& value)
        {
            std::vector<std::string> parts;
            std::size_t start = 0;
            while (true)
            {
                const std::size_t comma = value.find(',', start);
                if (comma == std::string::npos)
                {
                    parts.push_back(value.substr(start));
                    break;
                }
                parts.push_back(value.substr(start, comma - start));
                start = comma + 1;
            }
            return parts;
        }

        std::vector<SharpRuntime::bytecs> GetByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute,
            std::size_t expectedCount)
        {
            std::vector<SharpRuntime::bytecs> result(expectedCount, 0);
            std::string value;
            if (TryGetAttributeValue(node, attribute, value))
            {
                const std::vector<std::string> parts = SplitByComma(value);
                for (std::size_t i = 0; i < expectedCount; ++i)
                {
                    result[i] = System::Convert::ToByte(parts.at(i));
                }
            }
            return result;
        }

        std::vector<SharpRuntime::sbytecs> GetSignedByteDelimitedAttribute(const System::Xml::XmlNode* node, const std::string& attribute,
            std::size_t expectedCount)
        {
            std::vector<SharpRuntime::sbytecs> result(expectedCount, 0);
            std::string value;
            if (TryGetAttributeValue(node, attribute, value))
            {
                const std::vector<std::string> parts = SplitByComma(value);
                for (std::size_t i = 0; i < expectedCount; ++i)
                {
                    result[i] = System::Convert::ToSByte(parts.at(i));
                }
            }
            return result;
        }

        // --- Binary format -----------------------------------------------------------------------
        // Upstream reads a fixed-size byte buffer per block and reinterprets it via
        // Marshal.PtrToStructure onto a [StructLayout(LayoutKind.Explicit)] struct -- a
        // .NET-marshaling-specific mechanism. Reading each field individually, in the same
        // declared/offset order, via BinaryReader produces byte-for-byte identical field values
        // through a portable mechanism (see BitmapFontFileContent.hpp's header comment).

        HeaderBlock ReadHeaderBlockBinary(System::IO::BinaryReader& reader)
        {
            HeaderBlock header;
            header.B = reader.ReadByte();
            header.M = reader.ReadByte();
            header.F = reader.ReadByte();
            header.Version = reader.ReadByte();
            return header;
        }

        InfoBlock ReadInfoBlockBinary(System::IO::BinaryReader& reader)
        {
            InfoBlock info;
            info.FontSize = reader.ReadInt16();
            info.BitField = reader.ReadByte();
            info.CharSet = reader.ReadByte();
            info.StretchH = reader.ReadUInt16();
            info.AA = reader.ReadByte();
            info.PaddingUp = reader.ReadByte();
            info.PaddingRight = reader.ReadByte();
            info.PaddingDown = reader.ReadByte();
            info.PaddingLeft = reader.ReadByte();
            info.SpacingHoriz = reader.ReadSByte();
            info.SpacingVert = reader.ReadSByte();
            info.Outline = reader.ReadByte();
            return info;
        }

        CommonBlock ReadCommonBlockBinary(System::IO::BinaryReader& reader)
        {
            CommonBlock common;
            common.LineHeight = reader.ReadUInt16();
            common.Base = reader.ReadUInt16();
            common.ScaleW = reader.ReadUInt16();
            common.ScaleH = reader.ReadUInt16();
            common.Pages = reader.ReadUInt16();
            common.BitField = reader.ReadByte();
            common.AlphaChnl = reader.ReadByte();
            common.RedChnl = reader.ReadByte();
            common.GreenChnl = reader.ReadByte();
            common.BlueChnl = reader.ReadByte();
            return common;
        }

        CharacterBlock ReadCharacterBlockBinary(System::IO::BinaryReader& reader)
        {
            CharacterBlock character;
            character.ID = reader.ReadInt32();
            character.X = reader.ReadUInt16();
            character.Y = reader.ReadUInt16();
            character.Width = reader.ReadUInt16();
            character.Height = reader.ReadUInt16();
            character.XOffset = reader.ReadInt16();
            character.YOffset = reader.ReadInt16();
            character.XAdvance = reader.ReadInt16();
            character.Page = reader.ReadByte();
            character.Chnl = reader.ReadByte();
            return character;
        }

        KerningPairsBlock ReadKerningPairsBlockBinary(System::IO::BinaryReader& reader)
        {
            KerningPairsBlock kerning;
            kerning.First = reader.ReadUInt32();
            kerning.Second = reader.ReadUInt32();
            kerning.Amount = reader.ReadInt16();
            return kerning;
        }

        BitmapFontFileContent ReadBinary(System::IO::Stream& stream)
        {
            System::IO::BinaryReader reader(&stream);

            BitmapFontFileContent bmfFile;
            bmfFile.Header = ReadHeaderBlockBinary(reader);

            if (!bmfFile.Header.getIsValidProperty())
            {
                throw System::InvalidOperationException(
                    "The BMFFont file header is invalid, this does not appear to be a valid BMFont file");
            }

            System::IO::Stream* baseStream = reader.getBaseStreamProperty();
            while (baseStream->getPositionProperty() < baseStream->getLengthProperty())
            {
                const SharpRuntime::bytecs blockType = reader.ReadByte();
                const SharpRuntime::intcs blockSize = reader.ReadInt32();

                switch (blockType)
                {
                    case 1:
                    {
                        bmfFile.Info = ReadInfoBlockBinary(reader);
                        const SharpRuntime::intcs stringLen = blockSize - InfoBlock::StructSize;
                        std::string fontName = System::Text::Encoding::UTF8()->GetString(reader.ReadBytes(stringLen));
                        fontName.erase(std::remove(fontName.begin(), fontName.end(), '\0'), fontName.end());
                        bmfFile.FontName = fontName;
                        break;
                    }
                    case 2:
                        bmfFile.Common = ReadCommonBlockBinary(reader);
                        break;
                    case 3:
                    {
                        const std::string pagesBlob = System::Text::Encoding::UTF8()->GetString(reader.ReadBytes(blockSize));
                        std::size_t start = 0;
                        while (start < pagesBlob.size())
                        {
                            const std::size_t nul = pagesBlob.find('\0', start);
                            const std::size_t end = (nul == std::string::npos) ? pagesBlob.size() : nul;
                            if (end > start)
                            {
                                bmfFile.Pages.push_back(pagesBlob.substr(start, end - start));
                            }
                            start = end + 1;
                        }
                        break;
                    }
                    case 4:
                    {
                        const SharpRuntime::intcs characterCount = blockSize / CharacterBlock::StructSize;
                        for (SharpRuntime::intcs c = 0; c < characterCount; ++c)
                        {
                            bmfFile.Characters.push_back(ReadCharacterBlockBinary(reader));
                        }
                        break;
                    }
                    case 5:
                    {
                        const SharpRuntime::intcs kerningCount = blockSize / KerningPairsBlock::StructSize;
                        for (SharpRuntime::intcs k = 0; k < kerningCount; ++k)
                        {
                            bmfFile.Kernings.push_back(ReadKerningPairsBlockBinary(reader));
                        }
                        break;
                    }
                    default:
                        baseStream->Seek(blockSize, System::IO::SeekOrigin::Current);
                        break;
                }
            }

            return bmfFile;
        }

        // --- XML format ---------------------------------------------------------------------------

        void ReadInfoNode(BitmapFontFileContent& bmfFile, System::Xml::XmlNode* root)
        {
            // SelectSingleNode() returns a pointer into the live DOM tree (owned by `document`), not a
            // fresh allocation, despite its doc comment -- must NOT be wrapped in unique_ptr/deleted.
            System::Xml::XmlNode* node = root->SelectSingleNode("info");
            bmfFile.FontName = GetStringAttribute(node, "face");
            bmfFile.Info.FontSize = GetInt16Attribute(node, "size");

            const SharpRuntime::bytecs smooth = GetByteAttribute(node, "smooth");
            const SharpRuntime::bytecs unicode = GetByteAttribute(node, "unicode");
            const SharpRuntime::bytecs italic = GetByteAttribute(node, "italic");
            const SharpRuntime::bytecs bold = GetByteAttribute(node, "bold");
            const SharpRuntime::bytecs fixedHeight = GetByteAttribute(node, "fixedHeight");
            bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(
                (smooth << 7) | (unicode << 6) | (italic << 5) | (bold << 4) | (fixedHeight << 3));

            bmfFile.Info.CharSet = GetByteAttribute(node, "charSet");
            bmfFile.Info.StretchH = GetUInt16Attribute(node, "stretchH");
            bmfFile.Info.AA = GetByteAttribute(node, "aa");

            const std::vector<SharpRuntime::bytecs> paddingValues = GetByteDelimitedAttribute(node, "padding", 4);
            bmfFile.Info.PaddingUp = paddingValues[0];
            bmfFile.Info.PaddingRight = paddingValues[1];
            bmfFile.Info.PaddingDown = paddingValues[2];
            bmfFile.Info.PaddingLeft = paddingValues[3];

            const std::vector<SharpRuntime::sbytecs> spacingValues = GetSignedByteDelimitedAttribute(node, "spacing", 2);
            bmfFile.Info.SpacingHoriz = spacingValues[0];
            bmfFile.Info.SpacingVert = spacingValues[1];

            bmfFile.Info.Outline = GetByteAttribute(node, "outline");
        }

        void ReadCommonNode(BitmapFontFileContent& bmfFile, System::Xml::XmlNode* root)
        {
            System::Xml::XmlNode* node = root->SelectSingleNode("common");
            bmfFile.Common.LineHeight = GetUInt16Attribute(node, "lineHeight");
            bmfFile.Common.Base = GetUInt16Attribute(node, "base");
            bmfFile.Common.ScaleW = GetUInt16Attribute(node, "scaleW");
            bmfFile.Common.ScaleH = GetUInt16Attribute(node, "scaleH");
            bmfFile.Common.Pages = GetUInt16Attribute(node, "pages");

            const SharpRuntime::bytecs packed = GetByteAttribute(node, "packed");
            bmfFile.Common.BitField = static_cast<SharpRuntime::bytecs>(packed << 7);

            bmfFile.Common.AlphaChnl = GetByteAttribute(node, "alphaChnl");
            bmfFile.Common.RedChnl = GetByteAttribute(node, "redChnl");
            bmfFile.Common.GreenChnl = GetByteAttribute(node, "greenChnl");
            bmfFile.Common.BlueChnl = GetByteAttribute(node, "blueChnl");
        }

        void ReadPageNodes(BitmapFontFileContent& bmfFile, System::Xml::XmlNode* root)
        {
            const std::unique_ptr<System::Xml::XmlNodeList> nodes(root->SelectNodes("pages/page"));
            for (SharpRuntime::intcs i = 0; i < nodes->getCountProperty(); ++i)
            {
                bmfFile.Pages.push_back(GetStringAttribute(nodes->Item(i), "file"));
            }
        }

        void ReadCharacterNodes(BitmapFontFileContent& bmfFile, System::Xml::XmlNode* root)
        {
            const std::unique_ptr<System::Xml::XmlNodeList> nodes(root->SelectNodes("chars/char"));
            for (SharpRuntime::intcs i = 0; i < nodes->getCountProperty(); ++i)
            {
                System::Xml::XmlNode* node = nodes->Item(i);
                CharacterBlock character;
                character.ID = GetInt32Attribute(node, "id");
                character.X = GetUInt16Attribute(node, "x");
                character.Y = GetUInt16Attribute(node, "y");
                character.Width = GetUInt16Attribute(node, "width");
                character.Height = GetUInt16Attribute(node, "height");
                character.XOffset = GetInt16Attribute(node, "xoffset");
                character.YOffset = GetInt16Attribute(node, "yoffset");
                character.XAdvance = GetInt16Attribute(node, "xadvance");
                character.Page = GetByteAttribute(node, "page");
                character.Chnl = GetByteAttribute(node, "chnl");
                bmfFile.Characters.push_back(character);
            }
        }

        void ReadKerningNodes(BitmapFontFileContent& bmfFile, System::Xml::XmlNode* root)
        {
            const std::unique_ptr<System::Xml::XmlNodeList> nodes(root->SelectNodes("kernings/kerning"));
            for (SharpRuntime::intcs i = 0; i < nodes->getCountProperty(); ++i)
            {
                System::Xml::XmlNode* node = nodes->Item(i);
                KerningPairsBlock kerning;
                kerning.First = GetUInt32Attribute(node, "first");
                kerning.Second = GetUInt32Attribute(node, "second");
                kerning.Amount = GetInt16Attribute(node, "amount");
                bmfFile.Kernings.push_back(kerning);
            }
        }

        BitmapFontFileContent ReadXml(System::IO::Stream& stream)
        {
            BitmapFontFileContent bmfFile;

            // XML does not contain the header like binary so we manually create it.
            bmfFile.Header.B = static_cast<SharpRuntime::bytecs>('B');
            bmfFile.Header.M = static_cast<SharpRuntime::bytecs>('M');
            bmfFile.Header.F = static_cast<SharpRuntime::bytecs>('F');
            bmfFile.Header.Version = 3;

            std::vector<SharpRuntime::bytecs> remaining(static_cast<std::size_t>(stream.getLengthProperty() - stream.getPositionProperty()));
            SharpRuntime::intcs totalRead = 0;
            while (totalRead < static_cast<SharpRuntime::intcs>(remaining.size()))
            {
                const SharpRuntime::intcs read =
                    stream.Read(remaining.data(), totalRead, static_cast<SharpRuntime::intcs>(remaining.size()) - totalRead);
                if (read == 0)
                {
                    break;
                }
                totalRead += read;
            }
            const std::string xml(remaining.begin(), remaining.begin() + totalRead);

            System::Xml::XmlDocument document;
            document.LoadXml(xml);
            System::Xml::XmlElement* root = document.getDocumentElementProperty();

            ReadInfoNode(bmfFile, root);
            ReadCommonNode(bmfFile, root);
            ReadPageNodes(bmfFile, root);
            ReadCharacterNodes(bmfFile, root);
            ReadKerningNodes(bmfFile, root);

            return bmfFile;
        }

        // --- Text format --------------------------------------------------------------------------

        std::vector<std::string> GetTokens(const std::string& line)
        {
            std::vector<std::string> tokens;
            std::string currentToken;
            bool inQuotes = false;

            for (const char c : line)
            {
                if (c == ' ' && !inQuotes)
                {
                    if (!currentToken.empty())
                    {
                        tokens.push_back(currentToken);
                        currentToken.clear();
                    }
                }
                else if (c == '"')
                {
                    inQuotes = !inQuotes;
                }
                else
                {
                    currentToken.push_back(c);
                }
            }

            if (!currentToken.empty())
            {
                tokens.push_back(currentToken);
            }

            return tokens;
        }

        bool SplitKeyValue(const std::string& token, std::string& key, std::string& value)
        {
            const std::size_t eq = token.find('=');
            if (eq == std::string::npos)
            {
                return false;
            }
            key = token.substr(0, eq);
            value = token.substr(eq + 1);
            return true;
        }

        std::string StripQuotes(const std::string& value)
        {
            std::string result;
            result.reserve(value.size());
            for (const char c : value)
            {
                if (c != '"')
                {
                    result.push_back(c);
                }
            }
            return result;
        }

        void ReadInfoTokens(BitmapFontFileContent& bmfFile, const std::vector<std::string>& tokens)
        {
            std::string key, value;
            for (const std::string& token : tokens)
            {
                if (!SplitKeyValue(token, key, value))
                {
                    continue;
                }

                if (key == "face")
                {
                    bmfFile.FontName = StripQuotes(value);
                }
                else if (key == "size")
                {
                    bmfFile.Info.FontSize = System::Convert::ToInt16(value);
                }
                else if (key == "smooth")
                {
                    bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Info.BitField | (System::Convert::ToByte(value) << 7));
                }
                else if (key == "unicode")
                {
                    bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Info.BitField | (System::Convert::ToByte(value) << 6));
                }
                else if (key == "italic")
                {
                    bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Info.BitField | (System::Convert::ToByte(value) << 5));
                }
                else if (key == "bold")
                {
                    bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Info.BitField | (System::Convert::ToByte(value) << 4));
                }
                else if (key == "fixedHeight")
                {
                    bmfFile.Info.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Info.BitField | (System::Convert::ToByte(value) << 3));
                }
                else if (key == "stretchH")
                {
                    bmfFile.Info.StretchH = System::Convert::ToUInt16(value);
                }
                else if (key == "aa")
                {
                    bmfFile.Info.AA = System::Convert::ToByte(value);
                }
                else if (key == "padding")
                {
                    const std::vector<std::string> paddingValues = SplitByComma(value);
                    if (paddingValues.size() == 4)
                    {
                        bmfFile.Info.PaddingUp = System::Convert::ToByte(paddingValues[0]);
                        bmfFile.Info.PaddingRight = System::Convert::ToByte(paddingValues[1]);
                        bmfFile.Info.PaddingDown = System::Convert::ToByte(paddingValues[2]);
                        bmfFile.Info.PaddingLeft = System::Convert::ToByte(paddingValues[3]);
                    }
                }
                else if (key == "spacing")
                {
                    const std::vector<std::string> spacingValues = SplitByComma(value);
                    if (spacingValues.size() == 2)
                    {
                        bmfFile.Info.SpacingHoriz = System::Convert::ToSByte(spacingValues[0]);
                        bmfFile.Info.SpacingVert = System::Convert::ToSByte(spacingValues[1]);
                    }
                }
                else if (key == "outline")
                {
                    bmfFile.Info.Outline = System::Convert::ToByte(value);
                }
            }
        }

        void ReadCommonTokens(BitmapFontFileContent& bmfFile, const std::vector<std::string>& tokens)
        {
            std::string key, value;
            for (const std::string& token : tokens)
            {
                if (!SplitKeyValue(token, key, value))
                {
                    continue;
                }

                if (key == "lineHeight")
                {
                    bmfFile.Common.LineHeight = System::Convert::ToUInt16(value);
                }
                else if (key == "base")
                {
                    bmfFile.Common.Base = System::Convert::ToUInt16(value);
                }
                else if (key == "scaleW")
                {
                    bmfFile.Common.ScaleW = System::Convert::ToUInt16(value);
                }
                else if (key == "scaleH")
                {
                    bmfFile.Common.ScaleH = System::Convert::ToUInt16(value);
                }
                else if (key == "pages")
                {
                    bmfFile.Common.Pages = System::Convert::ToUInt16(value);
                }
                else if (key == "packed")
                {
                    bmfFile.Common.BitField = static_cast<SharpRuntime::bytecs>(bmfFile.Common.BitField | (System::Convert::ToByte(value) << 7));
                }
                else if (key == "alphaChnl")
                {
                    bmfFile.Common.AlphaChnl = System::Convert::ToByte(value);
                }
                else if (key == "redChnl")
                {
                    bmfFile.Common.RedChnl = System::Convert::ToByte(value);
                }
                else if (key == "greenChnl")
                {
                    bmfFile.Common.GreenChnl = System::Convert::ToByte(value);
                }
                else if (key == "blueChnl")
                {
                    bmfFile.Common.BlueChnl = System::Convert::ToByte(value);
                }
            }
        }

        void ReadPageTokens(BitmapFontFileContent& bmfFile, const std::vector<std::string>& tokens)
        {
            std::string key, value;
            for (const std::string& token : tokens)
            {
                if (!SplitKeyValue(token, key, value))
                {
                    continue;
                }
                if (key == "file")
                {
                    bmfFile.Pages.push_back(StripQuotes(value));
                }
            }
        }

        void ReadCharacterTokens(BitmapFontFileContent& bmfFile, const std::vector<std::string>& tokens)
        {
            CharacterBlock character;
            std::string key, value;
            for (const std::string& token : tokens)
            {
                if (!SplitKeyValue(token, key, value))
                {
                    continue;
                }

                if (key == "id")
                {
                    character.ID = System::Convert::ToInt32(value);
                }
                else if (key == "x")
                {
                    character.X = System::Convert::ToUInt16(value);
                }
                else if (key == "y")
                {
                    character.Y = System::Convert::ToUInt16(value);
                }
                else if (key == "width")
                {
                    character.Width = System::Convert::ToUInt16(value);
                }
                else if (key == "height")
                {
                    character.Height = System::Convert::ToUInt16(value);
                }
                else if (key == "xoffset")
                {
                    character.XOffset = System::Convert::ToInt16(value);
                }
                else if (key == "yoffset")
                {
                    character.YOffset = System::Convert::ToInt16(value);
                }
                else if (key == "xadvance")
                {
                    character.XAdvance = System::Convert::ToInt16(value);
                }
                else if (key == "page")
                {
                    character.Page = System::Convert::ToByte(value);
                }
                else if (key == "chnl")
                {
                    character.Chnl = System::Convert::ToByte(value);
                }
            }
            bmfFile.Characters.push_back(character);
        }

        void ReadKerningTokens(BitmapFontFileContent& bmfFile, const std::vector<std::string>& tokens)
        {
            KerningPairsBlock kerning;
            std::string key, value;
            for (const std::string& token : tokens)
            {
                if (!SplitKeyValue(token, key, value))
                {
                    continue;
                }

                if (key == "first")
                {
                    kerning.First = System::Convert::ToUInt32(value);
                }
                else if (key == "second")
                {
                    kerning.Second = System::Convert::ToUInt32(value);
                }
                else if (key == "amount")
                {
                    kerning.Amount = System::Convert::ToInt16(value);
                }
            }
            bmfFile.Kernings.push_back(kerning);
        }

        BitmapFontFileContent ReadText(System::IO::Stream& stream)
        {
            BitmapFontFileContent bmfFile;

            // Text does not contain the header like binary so we manually create it.
            bmfFile.Header.B = static_cast<SharpRuntime::bytecs>('B');
            bmfFile.Header.M = static_cast<SharpRuntime::bytecs>('M');
            bmfFile.Header.F = static_cast<SharpRuntime::bytecs>('F');
            bmfFile.Header.Version = 3;

            System::IO::StreamReader reader(&stream);

            while (reader.Peek() != -1)
            {
                const std::vector<std::string> tokens = GetTokens(reader.ReadLine());
                if (tokens.empty())
                {
                    continue;
                }

                if (tokens[0] == "info")
                {
                    ReadInfoTokens(bmfFile, std::vector<std::string>(tokens.begin() + 1, tokens.end()));
                }
                else if (tokens[0] == "common")
                {
                    ReadCommonTokens(bmfFile, std::vector<std::string>(tokens.begin() + 1, tokens.end()));
                }
                else if (tokens[0] == "page")
                {
                    ReadPageTokens(bmfFile, std::vector<std::string>(tokens.begin() + 1, tokens.end()));
                }
                else if (tokens[0] == "char")
                {
                    ReadCharacterTokens(bmfFile, std::vector<std::string>(tokens.begin() + 1, tokens.end()));
                }
                else if (tokens[0] == "kerning")
                {
                    ReadKerningTokens(bmfFile, std::vector<std::string>(tokens.begin() + 1, tokens.end()));
                }
            }

            return bmfFile;
        }
    }

    BitmapFontFileContent Read(System::IO::Stream& stream, const std::string& name)
    {
        const SharpRuntime::intcs position = stream.getPositionProperty();

        // Issue: MonoGame.Extended won't load XML format .fnt files if they begin with the byte
        // order mark. https://github.com/MonoGame-Extended/Monogame-Extended/issues/1073
        // A consumer might edit the BMFont file using a different library (e.g. SharpFNT.BitmapFont)
        // which could save it with a UTF-8 BOM preamble at the start of the file. Detect and skip
        // past it if present.
        SharpRuntime::bytecs buffer[3] = {0, 0, 0};
        const SharpRuntime::intcs bytesRead = stream.Read(buffer, 0, 3);
        if (bytesRead < 1)
        {
            throw System::InvalidOperationException("Stream is empty or unreadable");
        }

        SharpRuntime::intcs sig;
        if (bytesRead == 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF)
        {
            sig = stream.ReadByte();
            stream.Seek(-1, System::IO::SeekOrigin::Current);
        }
        else
        {
            sig = buffer[0];
            stream.setPositionProperty(position);
        }

        BitmapFontFileContent bmfFile;
        switch (sig)
        {
            // Binary header begins with [66, 77, 70, 3].
            case 66:
                bmfFile = ReadBinary(stream);
                break;

            // XML format begins with [60, 63, 120, 109].
            case 60:
                bmfFile = ReadXml(stream);
                break;

            // Text format begins with [105, 110, 102, 111].
            case 105:
                bmfFile = ReadText(stream);
                break;

            default:
                throw System::InvalidOperationException("This does not appear to be a valid BMFont file!");
        }

        bmfFile.Path = name;
        return bmfFile;
    }

    BitmapFontFileContent Read(const std::string& path)
    {
        System::IO::FileStream stream(path);
        return Read(stream, path);
    }
}
