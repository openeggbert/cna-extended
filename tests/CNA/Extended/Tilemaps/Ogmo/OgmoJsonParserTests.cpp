// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tilemaps/OgmoJsonParserTests.cs.
// Upstream's tests mostly call OgmoJsonParser::ParseFromFile/ParseFromStream end-to-end against a
// real GraphicsDevice (to load referenced tileset PNGs) -- this project has no headless
// GraphicsDevice test infrastructure (a standing, documented gap; see TiledTmxParserTests.cpp for
// the same limitation on the Tiled side). Instead, these tests exercise the two stages that stay
// fully headless: JSON deserialization into Document::OgmoLevel/OgmoProject (matching
// System::Text::Json::JsonSerializer::Deserialize<T>, the direct analogue of ParseMapXml) and
// Converters::Convert (document model -> TilemapData). Tileset PNG-dimension reading is exercised
// too, via a synthetic in-memory ExternalResourceResolver returning a minimal valid PNG header
// (24 bytes: signature + IHDR chunk with width/height) -- no live GraphicsDevice needed for this,
// since ReadImageDimensions only reads raw bytes.
#include "CNA/Extended/Tilemaps/Ogmo/Converters/OgmoTilemapDataConverter.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/OgmoColorParser.hpp"
#include "CNA/Extended/Tilemaps/Ogmo/OgmoJsonParser.hpp"

#include "System/IO/MemoryStream.hpp"
#include "System/Text/Json/JsonSerializer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps::Ogmo
{
    namespace
    {
        Document::OgmoLevel ParseLevel(const std::string& json)
        {
            return System::Text::Json::JsonSerializer::Deserialize<Document::OgmoLevel>(json);
        }

        Document::OgmoProject ParseProject(const std::string& json)
        {
            return System::Text::Json::JsonSerializer::Deserialize<Document::OgmoProject>(json);
        }

        const TilemapTileLayerData* FindTileLayer(const TilemapData& data, const std::string& name)
        {
            for (const std::unique_ptr<TilemapLayerData>& layer : data.Layers)
            {
                if (layer->Name == name)
                {
                    return dynamic_cast<const TilemapTileLayerData*>(layer.get());
                }
            }
            return nullptr;
        }

        const TilemapObjectLayerData* FindObjectLayer(const TilemapData& data, const std::string& name)
        {
            for (const std::unique_ptr<TilemapLayerData>& layer : data.Layers)
            {
                if (layer->Name == name)
                {
                    return dynamic_cast<const TilemapObjectLayerData*>(layer.get());
                }
            }
            return nullptr;
        }

        const TilemapPropertyData* FindProperty(const std::vector<TilemapPropertyData>& properties, const std::string& key)
        {
            for (const TilemapPropertyData& property : properties)
            {
                if (property.Key == key)
                {
                    return &property;
                }
            }
            return nullptr;
        }

        // 24-byte minimal PNG header (signature + IHDR chunk length/type/width/height), matching
        // what ReadImageDimensions actually reads -- the rest of a real PNG is irrelevant here.
        std::vector<SharpRuntime::bytecs> MakePngHeader(std::uint32_t width, std::uint32_t height)
        {
            std::vector<SharpRuntime::bytecs> header = {
                0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, // signature
                0x00, 0x00, 0x00, 0x0D,                         // IHDR length
                'I', 'H', 'D', 'R',                             // IHDR type
                static_cast<SharpRuntime::bytecs>((width >> 24) & 0xFF), static_cast<SharpRuntime::bytecs>((width >> 16) & 0xFF),
                static_cast<SharpRuntime::bytecs>((width >> 8) & 0xFF), static_cast<SharpRuntime::bytecs>(width & 0xFF),
                static_cast<SharpRuntime::bytecs>((height >> 24) & 0xFF), static_cast<SharpRuntime::bytecs>((height >> 16) & 0xFF),
                static_cast<SharpRuntime::bytecs>((height >> 8) & 0xFF), static_cast<SharpRuntime::bytecs>(height & 0xFF)};
            return header;
        }

        Content::ExternalResourceResolver MakePngResolver(std::uint32_t width, std::uint32_t height)
        {
            return [width, height](const std::string&) -> std::unique_ptr<System::IO::Stream>
            {
                const std::vector<SharpRuntime::bytecs> header = MakePngHeader(width, height);
                return std::make_unique<System::IO::MemoryStream>(header.data(), static_cast<SharpRuntime::intcs>(header.size()));
            };
        }
    }

    // --- OgmoColorParser ------------------------------------------------------------------

    TEST(OgmoColorParserTests, ParsesEightDigitHexWithAlpha)
    {
        const Color color = ParseColor("#11223344");
        EXPECT_EQ(color.getRProperty(), 0x11);
        EXPECT_EQ(color.getGProperty(), 0x22);
        EXPECT_EQ(color.getBProperty(), 0x33);
        EXPECT_EQ(color.getAProperty(), 0x44);
    }

    TEST(OgmoColorParserTests, EmptyStringReturnsWhite)
    {
        EXPECT_EQ(ParseColor(""), Color::White);
    }

    TEST(OgmoColorParserTests, MissingHashReturnsWhite)
    {
        EXPECT_EQ(ParseColor("11223344"), Color::White);
    }

    TEST(OgmoColorParserTests, WrongLengthReturnsWhite)
    {
        EXPECT_EQ(ParseColor("#112233"), Color::White);
    }

    TEST(OgmoColorParserTests, InvalidHexDigitsReturnWhite)
    {
        EXPECT_EQ(ParseColor("#GGGGGGGG"), Color::White);
    }

    // --- OgmoJsonParser::CanParse -----------------------------------------------------------

    TEST(OgmoJsonParserTests, CanParseAcceptsJsonExtension)
    {
        const OgmoJsonParser parser("/tmp/test_project.ogmo");
        EXPECT_TRUE(parser.CanParse("level.json"));
    }

    TEST(OgmoJsonParserTests, CanParseRejectsOtherExtensions)
    {
        const OgmoJsonParser parser("/tmp/test_project.ogmo");
        EXPECT_FALSE(parser.CanParse("level.txt"));
    }

    TEST(OgmoJsonParserTests, CanParseRejectsEmptyPath)
    {
        const OgmoJsonParser parser("/tmp/test_project.ogmo");
        EXPECT_FALSE(parser.CanParse(""));
    }

    TEST(OgmoJsonParserTests, SupportedExtensionsContainsJson)
    {
        const OgmoJsonParser parser("/tmp/test_project.ogmo");
        EXPECT_EQ(parser.getSupportedExtensionsProperty().size(), 1u);
        EXPECT_EQ(parser.getSupportedExtensionsProperty()[0], ".json");
    }

    // --- Converters::Convert: minimal / empty level -------------------------------------------

    TEST(OgmoTilemapDataConverterTests, MinimalLevelProducesCorrectGridDimensions)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 320,
            "height": 240,
            "offsetX": 0,
            "offsetY": 0,
            "layers": []
        })");
        const Document::OgmoProject project = ParseProject(R"({
            "ogmoVersion": "3.4.0",
            "name": "TestProject",
            "layerGridDefaultSize": {"x": 16, "y": 16},
            "layers": [],
            "tilesets": []
        })");

        const TilemapData data = Converters::Convert(level, project);

        EXPECT_EQ(data.Width, 20);
        EXPECT_EQ(data.Height, 15);
        EXPECT_EQ(data.TileWidth, 16);
        EXPECT_EQ(data.TileHeight, 16);
        EXPECT_EQ(data.Orientation, TilemapOrientation::Orthogonal);
        EXPECT_TRUE(data.Layers.empty());
    }

    TEST(OgmoTilemapDataConverterTests, ParsesBackgroundColor)
    {
        const Document::OgmoLevel level = ParseLevel(R"({"ogmoVersion": "3.4.0", "width": 16, "height": 16, "layers": []})");
        const Document::OgmoProject project = ParseProject(R"({
            "ogmoVersion": "3.4.0",
            "backgroundColor": "#282c34ff",
            "layerGridDefaultSize": {"x": 16, "y": 16},
            "layers": [],
            "tilesets": []
        })");

        const TilemapData data = Converters::Convert(level, project);

        ASSERT_TRUE(data.BackgroundColor.has_value());
        EXPECT_EQ(data.BackgroundColor->getRProperty(), 0x28);
        EXPECT_EQ(data.BackgroundColor->getGProperty(), 0x2c);
        EXPECT_EQ(data.BackgroundColor->getBProperty(), 0x34);
        EXPECT_EQ(data.BackgroundColor->getAProperty(), 0xff);
    }

    // --- Converters::Convert: level properties ------------------------------------------------

    TEST(OgmoTilemapDataConverterTests, StoresOgmoLevelProperties)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 640,
            "height": 480,
            "offsetX": 10,
            "offsetY": 20,
            "layers": []
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapPropertyData* version = FindProperty(data.Properties, "Ogmo_Version");
        ASSERT_NE(version, nullptr);
        EXPECT_EQ(version->StringValue, "3.4.0");

        const TilemapPropertyData* pixelWidth = FindProperty(data.Properties, "Ogmo_PixelWidth");
        ASSERT_NE(pixelWidth, nullptr);
        EXPECT_EQ(pixelWidth->IntValue, 640);

        const TilemapPropertyData* pixelHeight = FindProperty(data.Properties, "Ogmo_PixelHeight");
        ASSERT_NE(pixelHeight, nullptr);
        EXPECT_EQ(pixelHeight->IntValue, 480);

        const TilemapPropertyData* offsetX = FindProperty(data.Properties, "Ogmo_OffsetX");
        ASSERT_NE(offsetX, nullptr);
        EXPECT_EQ(offsetX->IntValue, 10);

        const TilemapPropertyData* offsetY = FindProperty(data.Properties, "Ogmo_OffsetY");
        ASSERT_NE(offsetY, nullptr);
        EXPECT_EQ(offsetY->IntValue, 20);
    }

    TEST(OgmoTilemapDataConverterTests, ConvertsCustomLevelValuesByJsonKind)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 16,
            "height": 16,
            "layers": [],
            "values": {"difficulty": 3, "gravity": 9.8, "isBoss": true, "name": "Level One"}
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapPropertyData* difficulty = FindProperty(data.Properties, "Level_difficulty");
        ASSERT_NE(difficulty, nullptr);
        EXPECT_EQ(difficulty->Type, TilemapPropertyType::Int);
        EXPECT_EQ(difficulty->IntValue, 3);

        const TilemapPropertyData* gravity = FindProperty(data.Properties, "Level_gravity");
        ASSERT_NE(gravity, nullptr);
        EXPECT_EQ(gravity->Type, TilemapPropertyType::Float);
        EXPECT_FLOAT_EQ(gravity->FloatValue, 9.8f);

        const TilemapPropertyData* isBoss = FindProperty(data.Properties, "Level_isBoss");
        ASSERT_NE(isBoss, nullptr);
        EXPECT_EQ(isBoss->Type, TilemapPropertyType::Bool);
        EXPECT_TRUE(isBoss->BoolValue);

        const TilemapPropertyData* name = FindProperty(data.Properties, "Level_name");
        ASSERT_NE(name, nullptr);
        EXPECT_EQ(name->Type, TilemapPropertyType::String);
        EXPECT_EQ(name->StringValue, "Level One");
    }

    // --- Converters::Convert: tile layers ----------------------------------------------------

    TEST(OgmoTilemapDataConverterTests, ConvertsTileLayerWith1DData)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 32,
            "height": 32,
            "layers": [
                {
                    "name": "Ground",
                    "_eid": "1234",
                    "gridCellWidth": 16,
                    "gridCellHeight": 16,
                    "gridCellsX": 2,
                    "gridCellsY": 2,
                    "tileset": "Tileset",
                    "data": [0, -1, -1, 3],
                    "exportMode": 0,
                    "arrayMode": 0
                }
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({
            "ogmoVersion": "3.4.0",
            "layerGridDefaultSize": {"x": 16, "y": 16},
            "layers": [],
            "tilesets": [
                {"label": "Tileset", "path": "tileset.png", "tileWidth": 16, "tileHeight": 16, "tileSeparationX": 0, "tileSeparationY": 0}
            ]
        })");

        const TilemapData data = Converters::Convert(level, project, "/tmp", MakePngResolver(64, 16));

        ASSERT_EQ(data.Tilesets.size(), 1u);
        EXPECT_EQ(data.Tilesets[0].FirstGlobalId, 1);
        ASSERT_TRUE(data.Tilesets[0].InlineData.has_value());
        EXPECT_EQ(data.Tilesets[0].InlineData->Columns, 4);

        const TilemapTileLayerData* layer = FindTileLayer(data, "Ground");
        ASSERT_NE(layer, nullptr);
        EXPECT_EQ(layer->Width, 2);
        EXPECT_EQ(layer->Height, 2);
        EXPECT_TRUE(layer->IsVisible);
        EXPECT_FLOAT_EQ(layer->Opacity, 1.0f);

        // Two non-negative entries (index 0 -> (0,0), index 3 -> (1,1)); the two -1 entries are skipped.
        ASSERT_EQ(layer->Tiles.size(), 2u);
        EXPECT_EQ(layer->Tiles[0].X, 0);
        EXPECT_EQ(layer->Tiles[0].Y, 0);
        EXPECT_EQ(layer->Tiles[0].GlobalId, 1); // firstGid(1) + localId(0)
        EXPECT_EQ(layer->Tiles[1].X, 1);
        EXPECT_EQ(layer->Tiles[1].Y, 1);
        EXPECT_EQ(layer->Tiles[1].GlobalId, 4); // firstGid(1) + localId(3)
    }

    TEST(OgmoTilemapDataConverterTests, ConvertsTileLayerWith2DData)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 32,
            "height": 16,
            "layers": [
                {
                    "name": "Ground",
                    "gridCellsX": 2,
                    "gridCellsY": 1,
                    "tileset": "Tileset",
                    "data2D": [[5, -1]],
                    "exportMode": 0,
                    "arrayMode": 1
                }
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({
            "ogmoVersion": "3.4.0",
            "layers": [],
            "tilesets": [
                {"label": "Tileset", "path": "tileset.png", "tileWidth": 16, "tileHeight": 16}
            ]
        })");

        const TilemapData data = Converters::Convert(level, project, "/tmp", MakePngResolver(16, 16));

        const TilemapTileLayerData* layer = FindTileLayer(data, "Ground");
        ASSERT_NE(layer, nullptr);
        ASSERT_EQ(layer->Tiles.size(), 1u);
        EXPECT_EQ(layer->Tiles[0].X, 0);
        EXPECT_EQ(layer->Tiles[0].Y, 0);
        EXPECT_EQ(layer->Tiles[0].GlobalId, 6); // firstGid(1) + localId(5)
    }

    TEST(OgmoTilemapDataConverterTests, UnknownTilesetNameProducesEmptyLayer)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 16,
            "height": 16,
            "layers": [
                {"name": "Ground", "gridCellsX": 1, "gridCellsY": 1, "tileset": "DoesNotExist", "data": [0]}
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapTileLayerData* layer = FindTileLayer(data, "Ground");
        ASSERT_NE(layer, nullptr);
        EXPECT_TRUE(layer->Tiles.empty());
    }

    // --- Converters::Convert: grid layers ----------------------------------------------------

    TEST(OgmoTilemapDataConverterTests, ConvertsGridLayerToOgmoGridValuesProperty)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 16,
            "height": 16,
            "layers": [
                {"name": "Collision", "gridCellsX": 2, "gridCellsY": 1, "grid": ["0", "1"], "arrayMode": 0}
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapTileLayerData* layer = FindTileLayer(data, "Collision");
        ASSERT_NE(layer, nullptr);

        const TilemapPropertyData* gridValues = FindProperty(layer->Properties, "Ogmo_GridValues");
        ASSERT_NE(gridValues, nullptr);
        EXPECT_EQ(gridValues->Type, TilemapPropertyType::String);
        EXPECT_NE(gridValues->StringValue.find("0"), std::string::npos);
        EXPECT_NE(gridValues->StringValue.find("1"), std::string::npos);

        const TilemapPropertyData* arrayMode = FindProperty(layer->Properties, "Ogmo_GridArrayMode");
        ASSERT_NE(arrayMode, nullptr);
        EXPECT_EQ(arrayMode->IntValue, 0);
    }

    // --- Converters::Convert: entity layers --------------------------------------------------

    TEST(OgmoTilemapDataConverterTests, ConvertsEntityLayerWithCustomValues)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 16,
            "height": 16,
            "layers": [
                {
                    "name": "Entities",
                    "entities": [
                        {
                            "name": "Player",
                            "id": 1,
                            "x": 64,
                            "y": 64,
                            "width": 16,
                            "height": 16,
                            "values": {"health": 100, "speed": 5.5}
                        }
                    ]
                }
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapObjectLayerData* layer = FindObjectLayer(data, "Entities");
        ASSERT_NE(layer, nullptr);
        ASSERT_EQ(layer->Objects.size(), 1u);

        const TilemapObjectData& player = *layer->Objects[0];
        EXPECT_EQ(player.Name, "Player");
        EXPECT_FLOAT_EQ(player.X, 64.0f);
        EXPECT_FLOAT_EQ(player.Y, 64.0f);

        const TilemapPropertyData* health = FindProperty(player.Properties, "health");
        ASSERT_NE(health, nullptr);
        EXPECT_EQ(health->Type, TilemapPropertyType::Int);
        EXPECT_EQ(health->IntValue, 100);

        const TilemapPropertyData* speed = FindProperty(player.Properties, "speed");
        ASSERT_NE(speed, nullptr);
        EXPECT_EQ(speed->Type, TilemapPropertyType::Float);
        EXPECT_FLOAT_EQ(speed->FloatValue, 5.5f);
    }

    // --- Converters::Convert: decal layers ---------------------------------------------------

    TEST(OgmoTilemapDataConverterTests, ConvertsDecalLayer)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 16,
            "height": 16,
            "layers": [
                {
                    "name": "Decals",
                    "folder": "decals/",
                    "decals": [
                        {"x": 10, "y": 20, "texture": "logo.png", "rotation": 0, "scaleX": 1, "scaleY": 1}
                    ]
                }
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({"ogmoVersion": "3.4.0", "layers": [], "tilesets": []})");

        const TilemapData data = Converters::Convert(level, project);

        const TilemapObjectLayerData* layer = FindObjectLayer(data, "Decals");
        ASSERT_NE(layer, nullptr);
        ASSERT_EQ(layer->Objects.size(), 1u);

        const TilemapObjectData& decal = *layer->Objects[0];
        EXPECT_FLOAT_EQ(decal.X, 10.0f);
        EXPECT_FLOAT_EQ(decal.Y, 20.0f);
        // Placeholder size, matching upstream's own hardcoded fallback for decals (no fixed size).
        const auto* rectangleDecal = dynamic_cast<const TilemapRectangleObjectData*>(&decal);
        ASSERT_NE(rectangleDecal, nullptr);
        EXPECT_FLOAT_EQ(rectangleDecal->Width, 16.0f);
        EXPECT_FLOAT_EQ(rectangleDecal->Height, 16.0f);

        const TilemapPropertyData* texture = FindProperty(decal.Properties, "Texture");
        ASSERT_NE(texture, nullptr);
        EXPECT_EQ(texture->StringValue, "logo.png");

        const TilemapPropertyData* folder = FindProperty(decal.Properties, "Folder");
        ASSERT_NE(folder, nullptr);
        EXPECT_EQ(folder->StringValue, "decals/");
    }

    // --- Document::ParseCoordPair ------------------------------------------------------------

    TEST(OgmoDocumentTests, ParseCoordPairWithTwoElementsReturnsPoint)
    {
        const nlohmann::ordered_json pair = nlohmann::ordered_json::array({3, 4});
        const std::optional<Point> result = Document::ParseCoordPair(pair);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->X, 3);
        EXPECT_EQ(result->Y, 4);
    }

    TEST(OgmoDocumentTests, ParseCoordPairWithOneElementReturnsEmpty)
    {
        const nlohmann::ordered_json pair = nlohmann::ordered_json::array({-1});
        const std::optional<Point> result = Document::ParseCoordPair(pair);
        EXPECT_FALSE(result.has_value());
    }

    TEST(OgmoTilemapDataConverterTests, ConvertsTileLayerWithDataCoords)
    {
        const Document::OgmoLevel level = ParseLevel(R"({
            "ogmoVersion": "3.4.0",
            "width": 32,
            "height": 16,
            "layers": [
                {
                    "name": "Ground",
                    "gridCellsX": 2,
                    "gridCellsY": 1,
                    "tileset": "Tileset",
                    "dataCoords": [[1, 0], [-1]],
                    "exportMode": 1,
                    "arrayMode": 0
                }
            ]
        })");
        const Document::OgmoProject project = ParseProject(R"({
            "ogmoVersion": "3.4.0",
            "layers": [],
            "tilesets": [
                {"label": "Tileset", "path": "tileset.png", "tileWidth": 16, "tileHeight": 16}
            ]
        })");

        // A 4-column, 16px-wide tileset image: 64px / 16px = 4 columns.
        const TilemapData data = Converters::Convert(level, project, "/tmp", MakePngResolver(64, 16));

        const TilemapTileLayerData* layer = FindTileLayer(data, "Ground");
        ASSERT_NE(layer, nullptr);
        ASSERT_EQ(layer->Tiles.size(), 1u);
        // coord (1,0) with 4 columns -> localId = 0*4 + 1 = 1; firstGid(1) + 1 = 2.
        EXPECT_EQ(layer->Tiles[0].GlobalId, 2);
    }
}
