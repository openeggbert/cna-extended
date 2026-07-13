// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Tilemaps/Tiled/Converters/TiledTilemapDataConverter.hpp"

#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"
#include "CNA/Extended/Tilemaps/Tiled/TiledColorParser.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"
#include "System/Convert.hpp"
#include "System/IO/Compression/CompressionMode.hpp"
#include "System/IO/Compression/DeflateStream.hpp"
#include "System/IO/Compression/GZipStream.hpp"
#include "System/IO/MemoryStream.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>

namespace CNA::Extended::Tilemaps::Tiled::Converters
{
    using Parsers::TilemapParseException;
    using System::IO::Compression::CompressionMode;
    using System::IO::Compression::DeflateStream;
    using System::IO::Compression::GZipStream;
    using System::IO::MemoryStream;

    namespace
    {
        constexpr std::uint32_t FlipHorizontallyFlag = 0x80000000u;
        constexpr std::uint32_t FlipVerticallyFlag = 0x40000000u;
        constexpr std::uint32_t FlipDiagonallyFlag = 0x20000000u;
        constexpr std::uint32_t FlipMask = 0xE0000000u;

        struct FlipResult
        {
            int Gid;
            TilemapTileFlipFlags Flags;
        };

        std::string ToLowerInvariant(const std::string& value)
        {
            std::string result = value;
            std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return result;
        }

        bool IsWhitespaceOnly(const std::string& value)
        {
            return std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c) != 0; });
        }

        // Strict (whole-string-must-match) int/float parsing, matching C#'s int.Parse/float.Parse
        // semantics -- std::from_chars (unlike std::stoi/std::stof) never silently accepts trailing
        // garbage. Implemented locally with std::from_chars rather than via System::Int32::Parse/
        // System::Single::Parse (used elsewhere in this project for the same purpose) because
        // System/Single.hpp's `class Single` and System/Convert.hpp's `using SharpRuntime::Single;`
        // (needed here for Convert::FromBase64String) collide when both are included in the same
        // translation unit -- a genuine sharp-runtime header-ordering bug, not something to work
        // around by editing sharp-runtime itself (sibling-repo rule).
        int ParseStrictInt(const std::string& text)
        {
            int value = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            if (result.ec != std::errc() || result.ptr != text.data() + text.size())
            {
                throw std::invalid_argument("not a valid integer: '" + text + "'");
            }
            return value;
        }

        float ParseStrictFloat(const std::string& text)
        {
            float value = 0.0f;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            if (result.ec != std::errc() || result.ptr != text.data() + text.size())
            {
                throw std::invalid_argument("not a valid float: '" + text + "'");
            }
            return value;
        }

        FlipResult ExtractFlipFlags(std::uint32_t rawGid)
        {
            const int gid = static_cast<int>(rawGid & ~FlipMask);
            TilemapTileFlipFlags flags = TilemapTileFlipFlags::None;

            if ((rawGid & FlipHorizontallyFlag) != 0)
            {
                flags = flags | TilemapTileFlipFlags::FlipHorizontally;
            }
            if ((rawGid & FlipVerticallyFlag) != 0)
            {
                flags = flags | TilemapTileFlipFlags::FlipVertically;
            }
            if ((rawGid & FlipDiagonallyFlag) != 0)
            {
                flags = flags | TilemapTileFlipFlags::FlipDiagonally;
            }

            return FlipResult{gid, flags};
        }

        std::vector<SharpRuntime::bytecs> ReadAllBytes(System::IO::Stream& stream)
        {
            std::vector<SharpRuntime::bytecs> result;
            SharpRuntime::bytecs buffer[4096];
            SharpRuntime::intcs read;
            while ((read = stream.Read(buffer, 0, static_cast<SharpRuntime::intcs>(sizeof(buffer)))) > 0)
            {
                result.insert(result.end(), buffer, buffer + read);
            }
            return result;
        }

        std::vector<SharpRuntime::bytecs> DecompressGzip(const std::vector<SharpRuntime::bytecs>& data, const std::string& layerName)
        {
            try
            {
                MemoryStream input(data.data(), static_cast<SharpRuntime::intcs>(data.size()));
                GZipStream gzip(&input, CompressionMode::Decompress);
                return ReadAllBytes(gzip);
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Failed to decompress GZip tile data in layer '" + layerName + "'.");
            }
        }

        std::vector<SharpRuntime::bytecs> DecompressZlib(const std::vector<SharpRuntime::bytecs>& data, const std::string& layerName)
        {
            if (data.size() < 6)
            {
                throw TilemapParseException(
                    "Tile layer '" + layerName + "' has ZLib data that is too short (" + std::to_string(data.size()) + " bytes).");
            }

            try
            {
                // Zlib format: 2-byte header + DEFLATE stream + 4-byte Adler-32 checksum; DeflateStream
                // wants raw DEFLATE only, so the input is trimmed to [2, size-4) before wrapping.
                MemoryStream input(data.data() + 2, static_cast<SharpRuntime::intcs>(data.size() - 6));
                DeflateStream deflate(&input, CompressionMode::Decompress);
                return ReadAllBytes(deflate);
            }
            catch (const TilemapParseException&)
            {
                throw;
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Failed to decompress ZLib tile data in layer '" + layerName + "'.");
            }
        }

        std::vector<SharpRuntime::bytecs> Decompress(
            const std::vector<SharpRuntime::bytecs>& data, const std::string& compression, const std::string& layerName)
        {
            const std::string lower = ToLowerInvariant(compression);
            if (lower == "gzip")
            {
                return DecompressGzip(data, layerName);
            }
            if (lower == "zlib")
            {
                return DecompressZlib(data, layerName);
            }
            if (lower == "zstd")
            {
                throw TilemapParseException("Tile layer '" + layerName +
                    "' uses Zstandard (zstd) compression, which is not supported. In Tiled, change the tile layer format to GZip or zlib.");
            }
            throw TilemapParseException("Tile layer '" + layerName + "' uses unsupported compression '" + compression + "'.");
        }

        std::vector<std::uint32_t> DecodeXmlGids(const std::vector<Document::TiledDataTileXml>& tiles, int width, int height)
        {
            std::vector<std::uint32_t> gids(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), 0u);

            for (std::size_t i = 0; i < tiles.size() && i < gids.size(); ++i)
            {
                gids[i] = tiles[i].Gid;
            }

            return gids;
        }

        std::vector<std::uint32_t> DecodeCsvGids(const std::string& csv, const std::string& layerName)
        {
            if (csv.empty() || IsWhitespaceOnly(csv))
            {
                return {};
            }

            std::vector<std::uint32_t> gids;
            std::string token;
            const auto flush = [&]()
            {
                if (token.empty())
                {
                    return;
                }
                try
                {
                    std::size_t consumed = 0;
                    const unsigned long value = std::stoul(token, &consumed, 10);
                    if (consumed != token.size())
                    {
                        throw std::invalid_argument("trailing garbage");
                    }
                    gids.push_back(static_cast<std::uint32_t>(value));
                }
                catch (const std::exception&)
                {
                    throw TilemapParseException("Tile layer '" + layerName + "' contains an invalid GID value '" + token + "'.");
                }
                token.clear();
            };

            for (const char ch : csv)
            {
                if (ch == ',' || ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t')
                {
                    flush();
                }
                else
                {
                    token.push_back(ch);
                }
            }
            flush();

            return gids;
        }

        std::vector<std::uint32_t> DecodeBase64Gids(const std::string& base64, const std::string& compression, const std::string& layerName)
        {
            if (base64.empty() || IsWhitespaceOnly(base64))
            {
                return {};
            }

            // Trim leading/trailing whitespace (Tiled formats the base64 payload across multiple lines).
            std::size_t begin = base64.find_first_not_of(" \t\r\n");
            std::size_t end = base64.find_last_not_of(" \t\r\n");
            const std::string trimmed = base64.substr(begin, end - begin + 1);

            std::vector<SharpRuntime::bytecs> bytes;
            try
            {
                bytes = System::Convert::FromBase64String(trimmed);
            }
            catch (const std::exception&)
            {
                throw TilemapParseException("Tile layer '" + layerName + "' contains invalid Base64 tile data.");
            }

            if (!compression.empty())
            {
                bytes = Decompress(bytes, compression, layerName);
            }

            if (bytes.size() % 4 != 0)
            {
                throw TilemapParseException(
                    "Tile layer '" + layerName + "' has " + std::to_string(bytes.size()) + " decoded bytes, which is not a multiple of 4.");
            }

            std::vector<std::uint32_t> gids(bytes.size() / 4);
            for (std::size_t i = 0; i < gids.size(); ++i)
            {
                std::uint32_t value;
                std::memcpy(&value, bytes.data() + i * 4, sizeof(value));
                gids[i] = value; // Host is little-endian (matches BitConverter.ToUInt32's default on all supported platforms).
            }

            return gids;
        }

        // Only called for the non-empty-encoding case: ConvertTileLayer() handles the "no
        // encoding" (plain XML <tile> elements) case directly, since only it has the layer's
        // width/height in scope for DecodeXmlGids's array sizing.
        std::vector<std::uint32_t> GetRawGids(const Document::TiledTileLayerDataXml& data, const std::string& layerName)
        {
            const std::string lower = ToLowerInvariant(data.Encoding);
            if (lower == "csv")
            {
                return DecodeCsvGids(data.Value, layerName);
            }
            if (lower == "base64")
            {
                return DecodeBase64Gids(data.Value, data.Compression, layerName);
            }

            throw TilemapParseException(
                "Tile layer '" + layerName + "' uses unsupported encoding '" + data.Encoding + "'. Supported encodings are: XML (no encoding), CSV, and Base64.");
        }

        std::vector<std::uint32_t> GetRawGids(
            const std::string& rawValue, const std::string& encoding, const std::string& compression, const std::string& layerName)
        {
            const std::string lower = ToLowerInvariant(encoding);
            if (lower == "csv")
            {
                return DecodeCsvGids(rawValue, layerName);
            }
            if (lower == "base64")
            {
                return DecodeBase64Gids(rawValue, compression, layerName);
            }

            throw TilemapParseException(
                "Tile layer '" + layerName + "' uses unsupported encoding '" + encoding + "'. Chunk data requires CSV or Base64 encoding.");
        }

        TilemapOrientation ConvertOrientation(const std::string& orientation)
        {
            const std::string lower = ToLowerInvariant(orientation);
            if (lower == "isometric")
            {
                return TilemapOrientation::Isometric;
            }
            if (lower == "staggered")
            {
                return TilemapOrientation::Staggered;
            }
            if (lower == "hexagonal")
            {
                return TilemapOrientation::Hexagonal;
            }
            return TilemapOrientation::Orthogonal;
        }

        TilemapStaggerAxis ConvertStaggerAxis(const std::string& staggerAxis)
        {
            return ToLowerInvariant(staggerAxis) == "x" ? TilemapStaggerAxis::X : TilemapStaggerAxis::Y;
        }

        TilemapStaggerIndex ConvertStaggerIndex(const std::string& staggerIndex)
        {
            return ToLowerInvariant(staggerIndex) == "even" ? TilemapStaggerIndex::Even : TilemapStaggerIndex::Odd;
        }

        void ParsePolyPoints(const std::string& pointsString, std::vector<Vector2>& target)
        {
            if (pointsString.empty() || IsWhitespaceOnly(pointsString))
            {
                return;
            }

            std::size_t pos = 0;
            while (pos < pointsString.size())
            {
                const std::size_t spaceIndex = pointsString.find(' ', pos);
                const std::string part = pointsString.substr(pos, spaceIndex == std::string::npos ? std::string::npos : spaceIndex - pos);
                pos = spaceIndex == std::string::npos ? pointsString.size() : spaceIndex + 1;

                if (part.empty())
                {
                    continue;
                }

                const std::size_t commaIndex = part.find(',');
                if (commaIndex == std::string::npos)
                {
                    continue;
                }

                try
                {
                    const float x = ParseStrictFloat(part.substr(0, commaIndex));
                    const float y = ParseStrictFloat(part.substr(commaIndex + 1));
                    target.emplace_back(x, y);
                }
                catch (const std::exception&)
                {
                    // Matches upstream's TryParse-based skip-on-failure behavior for individual points.
                }
            }
        }

        TilemapTextObjectHorizontalAlignment ConvertHorizontalAlignment(const std::string& halign)
        {
            const std::string lower = ToLowerInvariant(halign);
            if (lower == "center")
            {
                return TilemapTextObjectHorizontalAlignment::Center;
            }
            if (lower == "right")
            {
                return TilemapTextObjectHorizontalAlignment::Right;
            }
            if (lower == "justify")
            {
                return TilemapTextObjectHorizontalAlignment::Justify;
            }
            return TilemapTextObjectHorizontalAlignment::Left;
        }

        TilemapTextObjectVerticalAlignment ConvertVerticalAlignment(const std::string& valign)
        {
            const std::string lower = ToLowerInvariant(valign);
            if (lower == "center")
            {
                return TilemapTextObjectVerticalAlignment::Center;
            }
            if (lower == "bottom")
            {
                return TilemapTextObjectVerticalAlignment::Bottom;
            }
            return TilemapTextObjectVerticalAlignment::Top;
        }

        void ConvertProperties(const Document::TiledPropertiesXml* source, std::vector<TilemapPropertyData>& target)
        {
            if (source == nullptr || source->Properties.empty())
            {
                return;
            }

            for (const Document::TiledPropertyXml& prop : source->Properties)
            {
                if (prop.Name.empty())
                {
                    continue;
                }

                TilemapPropertyData entry;
                entry.Key = prop.Name;
                const std::string type = prop.Type.empty() ? "string" : ToLowerInvariant(prop.Type);

                if (type == "int")
                {
                    entry.Type = TilemapPropertyType::Int;
                    entry.IntValue = ParseStrictInt(prop.Value.empty() ? "0" : prop.Value);
                }
                else if (type == "float")
                {
                    entry.Type = TilemapPropertyType::Float;
                    entry.FloatValue = ParseStrictFloat(prop.Value.empty() ? "0" : prop.Value);
                }
                else if (type == "bool")
                {
                    entry.Type = TilemapPropertyType::Bool;
                    entry.BoolValue = (prop.Value.empty() ? "false" : prop.Value) == "true";
                }
                else if (type == "color")
                {
                    entry.Type = TilemapPropertyType::Color;
                    const std::optional<Color> parsed = Parse(prop.Value.empty() ? "#00000000" : prop.Value);
                    entry.ColorValue = parsed.value_or(Color(0, 0, 0, 0));
                }
                else if (type == "file")
                {
                    entry.Type = TilemapPropertyType::File;
                    entry.StringValue = prop.Value;
                }
                else if (type == "object")
                {
                    entry.Type = TilemapPropertyType::Object;
                    try
                    {
                        entry.IntValue = ParseStrictInt(prop.Value);
                    }
                    catch (const std::exception&)
                    {
                        entry.IntValue = 0;
                    }
                }
                else
                {
                    entry.Type = TilemapPropertyType::String;
                    entry.StringValue = prop.Value;
                }

                target.push_back(entry);
            }
        }

        std::unique_ptr<TilemapObjectData> ConvertObject(const Document::TiledObjectXml& obj)
        {
            std::unique_ptr<TilemapObjectData> data;

            if (obj.Gid > 0)
            {
                const FlipResult flip = ExtractFlipFlags(obj.Gid);
                auto tileData = std::make_unique<TilemapTileObjectData>();
                tileData->GlobalId = flip.Gid;
                tileData->FlipFlags = flip.Flags;
                tileData->Width = obj.Width;
                tileData->Height = obj.Height;
                data = std::move(tileData);
            }
            else if (obj.Text)
            {
                auto textData = std::make_unique<TilemapTextObjectData>();
                textData->Width = obj.Width;
                textData->Height = obj.Height;
                textData->Text = obj.Text->Value;
                textData->FontFamily = obj.Text->FontFamily.empty() ? "sans-serif" : obj.Text->FontFamily;
                textData->PixelSize = obj.Text->PixelSize > 0 ? obj.Text->PixelSize : 16;
                textData->WordWrap = obj.Text->Wrap != 0;
                textData->Color = Parse(obj.Text->Color).value_or(Color(0, 0, 0, 255));
                textData->Bold = obj.Text->Bold != 0;
                textData->Italic = obj.Text->Italic != 0;
                textData->Underline = obj.Text->Underline != 0;
                textData->Strikethrough = obj.Text->Strikeout != 0;
                textData->HorizontalAlign = ConvertHorizontalAlignment(obj.Text->HAlign);
                textData->VerticalAlign = ConvertVerticalAlignment(obj.Text->VAlign);
                data = std::move(textData);
            }
            else if (obj.Ellipse)
            {
                auto ellipseData = std::make_unique<TilemapEllipseObjectData>();
                ellipseData->Width = obj.Width;
                ellipseData->Height = obj.Height;
                data = std::move(ellipseData);
            }
            else if (obj.Point)
            {
                data = std::make_unique<TilemapPointObjectData>();
            }
            else if (obj.Polygon)
            {
                auto polygonData = std::make_unique<TilemapPolygonObjectData>();
                ParsePolyPoints(obj.Polygon->Points, polygonData->Points);
                data = std::move(polygonData);
            }
            else if (obj.Polyline)
            {
                auto polylineData = std::make_unique<TilemapPolylineObjectData>();
                ParsePolyPoints(obj.Polyline->Points, polylineData->Points);
                data = std::move(polylineData);
            }
            else
            {
                auto rectData = std::make_unique<TilemapRectangleObjectData>();
                rectData->Width = obj.Width;
                rectData->Height = obj.Height;
                data = std::move(rectData);
            }

            data->Id = obj.Id;
            data->Name = obj.Name;
            data->Class = obj.Class.empty() ? obj.Type : obj.Class;
            data->X = obj.X;
            data->Y = obj.Y;
            data->Rotation = obj.Rotation * (Microsoft::Xna::Framework::MathHelper::Pi / 180.0f);
            data->IsVisible = obj.Visible != 0;

            ConvertProperties(obj.Properties.get(), data->Properties);

            return data;
        }

        void ApplyLayerBase(const Document::TiledLayerXml& source, TilemapLayerData& target)
        {
            target.Name = source.Name;
            target.Class = source.Class;
            target.IsVisible = source.Visible != 0;
            target.Opacity = source.Opacity;
            target.OffsetX += source.OffsetX;
            target.OffsetY += source.OffsetY;
            target.ParallaxX = source.ParallaxX;
            target.ParallaxY = source.ParallaxY;

            if (!source.TintColor.empty() && !IsWhitespaceOnly(source.TintColor))
            {
                target.TintColor = Parse(source.TintColor);
            }

            ConvertProperties(source.Properties.get(), target.Properties);
        }

        std::unique_ptr<TilemapLayerData> ConvertGroupLayer(const Document::TiledGroupLayerXml& groupLayer, const TilemapData& mapData);
        std::unique_ptr<TilemapLayerData> ConvertLayer(const Document::TiledLayerXml& layer, const TilemapData& mapData);

        void ConvertLayers(
            const std::vector<std::unique_ptr<Document::TiledLayerXml>>& xmlLayers, std::vector<std::unique_ptr<TilemapLayerData>>& outputLayers,
            const TilemapData& mapData)
        {
            for (const std::unique_ptr<Document::TiledLayerXml>& layer : xmlLayers)
            {
                if (std::unique_ptr<TilemapLayerData> layerData = ConvertLayer(*layer, mapData))
                {
                    outputLayers.push_back(std::move(layerData));
                }
            }
        }

        std::unique_ptr<TilemapTileLayerData> ConvertChunkedTileLayer(const Document::TiledTileLayerXml& tileLayer, const TilemapData& mapData)
        {
            int minTileX = std::numeric_limits<int>::max();
            int minTileY = std::numeric_limits<int>::max();
            int maxTileX = std::numeric_limits<int>::min();
            int maxTileY = std::numeric_limits<int>::min();

            for (const Document::TiledChunkXml& chunk : tileLayer.Data->Chunks)
            {
                minTileX = std::min(minTileX, chunk.X);
                minTileY = std::min(minTileY, chunk.Y);
                maxTileX = std::max(maxTileX, chunk.X + chunk.Width);
                maxTileY = std::max(maxTileY, chunk.Y + chunk.Height);
            }

            const int layerWidth = maxTileX - minTileX;
            const int layerHeight = maxTileY - minTileY;

            auto data = std::make_unique<TilemapTileLayerData>();
            data->Width = layerWidth;
            data->Height = layerHeight;
            // OffsetX/OffsetY combine with the layer's editor offset in ApplyLayerBase; store the
            // pixel origin here so TilemapFactory applies it as an additional offset.
            data->OffsetX = static_cast<float>(minTileX * mapData.TileWidth);
            data->OffsetY = static_cast<float>(minTileY * mapData.TileHeight);

            for (const Document::TiledChunkXml& chunk : tileLayer.Data->Chunks)
            {
                const std::vector<std::uint32_t> rawGids =
                    GetRawGids(chunk.Value, tileLayer.Data->Encoding, tileLayer.Data->Compression, tileLayer.Name);

                const int baseX = chunk.X - minTileX;
                const int baseY = chunk.Y - minTileY;

                for (std::size_t i = 0; i < rawGids.size(); ++i)
                {
                    const std::uint32_t rawGid = rawGids[i];
                    if (rawGid == 0)
                    {
                        continue;
                    }

                    const FlipResult flip = ExtractFlipFlags(rawGid);
                    const auto x = static_cast<std::uint16_t>(baseX + static_cast<int>(i) % chunk.Width);
                    const auto y = static_cast<std::uint16_t>(baseY + static_cast<int>(i) / chunk.Width);
                    data->Tiles.emplace_back(x, y, flip.Gid, flip.Flags);
                }
            }

            return data;
        }

        std::unique_ptr<TilemapTileLayerData> ConvertTileLayer(const Document::TiledTileLayerXml& tileLayer, const TilemapData& mapData)
        {
            const int layerWidth = tileLayer.Width > 0 ? tileLayer.Width : mapData.Width;
            const int layerHeight = tileLayer.Height > 0 ? tileLayer.Height : mapData.Height;

            if (!tileLayer.Data)
            {
                auto data = std::make_unique<TilemapTileLayerData>();
                data->Width = layerWidth;
                data->Height = layerHeight;
                return data;
            }

            const bool isChunked = !tileLayer.Data->Chunks.empty();
            if (isChunked)
            {
                return ConvertChunkedTileLayer(tileLayer, mapData);
            }

            auto data = std::make_unique<TilemapTileLayerData>();
            data->Width = layerWidth;
            data->Height = layerHeight;

            std::vector<std::uint32_t> rawGids;
            if (tileLayer.Data->Encoding.empty())
            {
                rawGids = DecodeXmlGids(tileLayer.Data->Tiles, layerWidth, layerHeight);
            }
            else
            {
                rawGids = GetRawGids(*tileLayer.Data, tileLayer.Name);
            }

            for (std::size_t i = 0; i < rawGids.size(); ++i)
            {
                const std::uint32_t rawGid = rawGids[i];
                if (rawGid == 0)
                {
                    continue;
                }

                const FlipResult flip = ExtractFlipFlags(rawGid);
                const auto x = static_cast<std::uint16_t>(static_cast<int>(i) % layerWidth);
                const auto y = static_cast<std::uint16_t>(static_cast<int>(i) / layerWidth);
                data->Tiles.emplace_back(x, y, flip.Gid, flip.Flags);
            }

            return data;
        }

        std::unique_ptr<TilemapObjectLayerData> ConvertObjectLayer(const Document::TiledObjectLayerXml& objectLayer)
        {
            auto data = std::make_unique<TilemapObjectLayerData>();
            data->DrawOrder =
                ToLowerInvariant(objectLayer.DrawOrder) == "index" ? TilemapObjectDrawOrder::Index : TilemapObjectDrawOrder::TopDown;

            for (const std::unique_ptr<Document::TiledObjectXml>& obj : objectLayer.Objects)
            {
                if (std::unique_ptr<TilemapObjectData> objData = ConvertObject(*obj))
                {
                    data->Objects.push_back(std::move(objData));
                }
            }

            return data;
        }

        std::unique_ptr<TilemapImageLayerData> ConvertImageLayer(const Document::TiledImageLayerXml& imageLayer)
        {
            if (!imageLayer.Image || imageLayer.Image->Source.empty())
            {
                return nullptr;
            }

            auto data = std::make_unique<TilemapImageLayerData>();
            data->TexturePath = imageLayer.Image->Source;
            data->RepeatX = imageLayer.RepeatX != 0;
            data->RepeatY = imageLayer.RepeatY != 0;
            return data;
        }

        std::unique_ptr<TilemapLayerData> ConvertGroupLayer(const Document::TiledGroupLayerXml& groupLayer, const TilemapData& mapData)
        {
            auto data = std::make_unique<TilemapGroupLayerData>();
            ConvertLayers(groupLayer.Layers, data->Layers, mapData);
            return data;
        }

        std::unique_ptr<TilemapLayerData> ConvertLayer(const Document::TiledLayerXml& layer, const TilemapData& mapData)
        {
            std::unique_ptr<TilemapLayerData> data;

            if (const auto* tileLayer = dynamic_cast<const Document::TiledTileLayerXml*>(&layer))
            {
                data = ConvertTileLayer(*tileLayer, mapData);
            }
            else if (const auto* objectLayer = dynamic_cast<const Document::TiledObjectLayerXml*>(&layer))
            {
                data = ConvertObjectLayer(*objectLayer);
            }
            else if (const auto* imageLayer = dynamic_cast<const Document::TiledImageLayerXml*>(&layer))
            {
                data = ConvertImageLayer(*imageLayer);
            }
            else if (const auto* groupLayer = dynamic_cast<const Document::TiledGroupLayerXml*>(&layer))
            {
                data = ConvertGroupLayer(*groupLayer, mapData);
            }
            else
            {
                return nullptr;
            }

            if (data)
            {
                ApplyLayerBase(layer, *data);
            }

            return data;
        }

        void ConvertTileEntries(const std::vector<std::unique_ptr<Document::TiledTileXml>>& tiledTiles, std::vector<TilemapTileEntryData>& entries)
        {
            for (const std::unique_ptr<Document::TiledTileXml>& tile : tiledTiles)
            {
                TilemapTileEntryData entry;
                entry.LocalId = tile->Id;
                entry.Class = !tile->Class.empty() ? tile->Class : tile->Type;
                entry.Probability = tile->Probability <= 0.0f ? 1.0f : tile->Probability;
                entry.ImagePath = tile->Image ? tile->Image->Source : std::string();

                ConvertProperties(tile->Properties.get(), entry.Properties);

                if (tile->Animation && !tile->Animation->Frames.empty())
                {
                    TilemapAnimationData animation;
                    for (const Document::TiledAnimationFrameXml& frame : tile->Animation->Frames)
                    {
                        // Tiled stores duration in milliseconds; the runtime wants seconds.
                        TilemapAnimationFrameData frameData;
                        frameData.TileId = frame.TileId;
                        frameData.Duration = static_cast<float>(frame.Duration) / 1000.0f;
                        animation.Frames.push_back(frameData);
                    }
                    entry.Animation = animation;
                }

                if (tile->ObjectGroup)
                {
                    for (const std::unique_ptr<Document::TiledObjectXml>& obj : tile->ObjectGroup->Objects)
                    {
                        if (std::unique_ptr<TilemapObjectData> objData = ConvertObject(*obj))
                        {
                            entry.CollisionObjects.push_back(std::move(objData));
                        }
                    }
                }

                entries.push_back(std::move(entry));
            }
        }

        TilemapTilesetData ConvertTilesetData(const Document::TiledTilesetXml& xml)
        {
            TilemapTilesetData data;
            data.Name = xml.Name;
            data.TexturePath = xml.Image ? xml.Image->Source : std::string();
            data.TileWidth = xml.TileWidth;
            data.TileHeight = xml.TileHeight;
            data.TileCount = xml.TileCount;
            data.Columns = xml.Columns;
            data.Spacing = xml.Spacing;
            data.Margin = xml.Margin;
            data.DrawOffsetX = xml.TileOffset ? static_cast<float>(xml.TileOffset->X) : 0.0f;
            data.DrawOffsetY = xml.TileOffset ? static_cast<float>(xml.TileOffset->Y) : 0.0f;

            ConvertProperties(xml.Properties.get(), data.Properties);
            ConvertTileEntries(xml.Tiles, data.Tiles);

            return data;
        }

        void ConvertTilesets(const Document::TiledMapXml& map, TilemapData& data)
        {
            for (const std::unique_ptr<Document::TiledTilesetRefXml>& tilesetRef : map.Tilesets)
            {
                TilemapTilesetEntry entry;
                entry.FirstGlobalId = tilesetRef->FirstGlobalId;

                // TilesetData is populated when external TSX files have been pre-loaded (runtime
                // path, see TiledTmxParser::LoadExternalTilesets). When it is null and Source is
                // set, this is an unresolved external reference (content-pipeline path only).
                const bool isExternalUnresolved = !tilesetRef->Source.empty() && !tilesetRef->TilesetData;

                if (isExternalUnresolved)
                {
                    entry.IsExternal = true;
                    entry.ExternalPath = tilesetRef->Source;
                }
                else
                {
                    const Document::TiledTilesetXml& tilesetXml = tilesetRef->TilesetData ? *tilesetRef->TilesetData : *tilesetRef;
                    entry.IsExternal = false;
                    entry.InlineData = ConvertTilesetData(tilesetXml);
                }

                data.Tilesets.push_back(std::move(entry));
            }
        }
    }

    TilemapData Convert(const Document::TiledMapXml& map)
    {
        TilemapData data;

        data.Width = map.Width;
        data.Height = map.Height;
        data.TileWidth = map.TileWidth;
        data.TileHeight = map.TileHeight;
        data.Orientation = ConvertOrientation(map.Orientation);
        data.StaggerAxis = ConvertStaggerAxis(map.StaggerAxis);
        data.StaggerIndex = ConvertStaggerIndex(map.StaggerIndex);
        data.HexSideLength = map.HexSideLength;
        data.ParallaxOriginX = map.ParallaxOriginX;
        data.ParallaxOriginY = map.ParallaxOriginY;

        if (!map.BackgroundColor.empty() && !IsWhitespaceOnly(map.BackgroundColor))
        {
            data.BackgroundColor = Parse(map.BackgroundColor);
        }

        ConvertProperties(map.Properties.get(), data.Properties);
        ConvertTilesets(map, data);
        ConvertLayers(map.Layers, data.Layers, data);

        return data;
    }
}
