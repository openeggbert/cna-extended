// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tilemaps/LDtkJsonParserTests.cs.
// Upstream's tests are `[Collection("GraphicsTest")]` and mostly call
// LDtkJsonParser::ParseFromFile/ParseFromStream end-to-end against a real GraphicsDevice (to load
// the fixture's referenced tileset PNG) -- this project has no headless GraphicsDevice test
// infrastructure (a standing, documented gap; see TiledTmxParserTests.cpp for the same
// limitation, and the same headless-testing strategy this file reuses). Instead, these tests
// exercise the parts that stay fully headless: parsing LDtk JSON into the Document model
// (`nlohmann::ordered_json::parse(...).get<Document::LDtkProject>()`) and
// `Converters::Convert` (Document model -> TilemapData) -- plus the 3 upstream tests that
// genuinely don't need a GraphicsDevice at all (`SupportedExtensions`/`CanParse`). Fixture JSON
// below is hand-authored (not the upstream 1620-line "all features" fixture, which mixes in many
// GraphicsDevice-only assertions) covering a Tiles layer, an Entities layer with typed field
// instances, an IntGrid layer, and an inline tileset definition.
#include "CNA/Extended/Tilemaps/LDtk/Converters/LDtkTilemapDataConverter.hpp"
#include "CNA/Extended/Tilemaps/LDtk/LDtkColorParser.hpp"
#include "CNA/Extended/Tilemaps/LDtk/LDtkJsonParser.hpp"

#include "CNA/Extended/Tilemaps/Parsers/TilemapParseException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps::LDtk
{
    namespace
    {
        const char* const kProjectJson = R"JSON({
            "iid": "proj-iid",
            "jsonVersion": "1.5.3",
            "appBuildId": 1,
            "nextUid": 100,
            "identifierStyle": "Capitalize",
            "worldLayout": "Free",
            "defaultGridSize": 16,
            "defaultPivotX": 0,
            "defaultPivotY": 0,
            "bgColor": "#40465B",
            "defaultLevelBgColor": "#696A79",
            "externalLevels": false,
            "defs": {
                "layers": [],
                "entities": [],
                "tilesets": [
                    {
                        "identifier": "Tileset",
                        "uid": 5,
                        "relPath": "tileset.png",
                        "pxWid": 64,
                        "pxHei": 64,
                        "tileGridSize": 16,
                        "spacing": 0,
                        "padding": 0,
                        "__cWid": 4,
                        "__cHei": 4
                    }
                ],
                "enums": [],
                "externalEnums": [],
                "levelFields": []
            },
            "worlds": [],
            "toc": [],
            "levels": [
                {
                    "identifier": "Level_0",
                    "iid": "level-iid",
                    "uid": 1,
                    "worldX": 0,
                    "worldY": 0,
                    "worldDepth": 0,
                    "pxWid": 32,
                    "pxHei": 32,
                    "__bgColor": "#696A79",
                    "bgColor": null,
                    "bgRelPath": null,
                    "bgPos": null,
                    "bgPivotX": 0.5,
                    "bgPivotY": 0.5,
                    "externalRelPath": null,
                    "fieldInstances": [],
                    "__neighbours": [],
                    "layerInstances": [
                        {
                            "__identifier": "Entities",
                            "__type": "Entities",
                            "__cWid": 2,
                            "__cHei": 2,
                            "__gridSize": 16,
                            "__opacity": 1.0,
                            "__pxTotalOffsetX": 0,
                            "__pxTotalOffsetY": 0,
                            "__tilesetDefUid": null,
                            "iid": "layer-entities-iid",
                            "levelId": 1,
                            "layerDefUid": 2,
                            "pxOffsetX": 0,
                            "pxOffsetY": 0,
                            "visible": true,
                            "__parallaxFactorX": 0,
                            "__parallaxFactorY": 0,
                            "gridTiles": [],
                            "autoLayerTiles": [],
                            "intGridCsv": [],
                            "entityInstances": [
                                {
                                    "__identifier": "Player",
                                    "iid": "entity-iid",
                                    "defUid": 10,
                                    "px": [16, 16],
                                    "width": 16,
                                    "height": 16,
                                    "__pivot": [0.5, 0.5],
                                    "__tags": ["hero"],
                                    "fieldInstances": [
                                        {
                                            "__identifier": "Health",
                                            "__type": "Int",
                                            "__value": 100,
                                            "defUid": 20,
                                            "realEditorValues": []
                                        },
                                        {
                                            "__identifier": "Name",
                                            "__type": "String",
                                            "__value": "Hero",
                                            "defUid": 21,
                                            "realEditorValues": []
                                        }
                                    ]
                                }
                            ]
                        },
                        {
                            "__identifier": "IntGrid",
                            "__type": "IntGrid",
                            "__cWid": 2,
                            "__cHei": 2,
                            "__gridSize": 16,
                            "__opacity": 1.0,
                            "__pxTotalOffsetX": 0,
                            "__pxTotalOffsetY": 0,
                            "__tilesetDefUid": null,
                            "iid": "layer-intgrid-iid",
                            "levelId": 1,
                            "layerDefUid": 3,
                            "pxOffsetX": 0,
                            "pxOffsetY": 0,
                            "visible": true,
                            "__parallaxFactorX": 0,
                            "__parallaxFactorY": 0,
                            "gridTiles": [],
                            "autoLayerTiles": [],
                            "entityInstances": [],
                            "intGridCsv": [1, 1, 0, 2]
                        },
                        {
                            "__identifier": "Tiles",
                            "__type": "Tiles",
                            "__cWid": 2,
                            "__cHei": 2,
                            "__gridSize": 16,
                            "__opacity": 1.0,
                            "__pxTotalOffsetX": 0,
                            "__pxTotalOffsetY": 0,
                            "__tilesetDefUid": 5,
                            "iid": "layer-tiles-iid",
                            "levelId": 1,
                            "layerDefUid": 4,
                            "pxOffsetX": 0,
                            "pxOffsetY": 0,
                            "visible": true,
                            "__parallaxFactorX": 0,
                            "__parallaxFactorY": 0,
                            "entityInstances": [],
                            "intGridCsv": [],
                            "gridTiles": [
                                { "px": [0, 0], "src": [0, 0], "f": 0, "t": 0, "d": [0, 0], "a": 1.0 },
                                { "px": [16, 0], "src": [16, 0], "f": 1, "t": 1, "d": [1, 0], "a": 1.0 }
                            ],
                            "autoLayerTiles": []
                        }
                    ]
                }
            ]
        })JSON";

        Document::LDtkProject ParseProject(const std::string& jsonText)
        {
            return nlohmann::ordered_json::parse(jsonText).get<Document::LDtkProject>();
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

        const TilemapDataLayerData* FindDataLayer(const TilemapData& data, const std::string& name)
        {
            for (const std::unique_ptr<TilemapLayerData>& layer : data.Layers)
            {
                if (layer->Name == name)
                {
                    return dynamic_cast<const TilemapDataLayerData*>(layer.get());
                }
            }
            return nullptr;
        }

        const TilemapPropertyData* FindProperty(const std::vector<TilemapPropertyData>& properties, const std::string& key)
        {
            for (const TilemapPropertyData& prop : properties)
            {
                if (prop.Key == key)
                {
                    return &prop;
                }
            }
            return nullptr;
        }
    }

    // --- LDtkColorParser ------------------------------------------------------------------------

    TEST(LDtkColorParserTests, ParseValidHexReturnsOpaqueColor)
    {
        const Color color = ParseColor("#FF8000");
        EXPECT_EQ(color.getRProperty(), 255);
        EXPECT_EQ(color.getGProperty(), 128);
        EXPECT_EQ(color.getBProperty(), 0);
        EXPECT_EQ(color.getAProperty(), 255);
    }

    TEST(LDtkColorParserTests, ParseEmptyStringReturnsTransparent)
    {
        EXPECT_EQ(ParseColor(""), Color::Transparent);
    }

    TEST(LDtkColorParserTests, ParseMissingHashReturnsTransparent)
    {
        EXPECT_EQ(ParseColor("FF8000"), Color::Transparent);
    }

    TEST(LDtkColorParserTests, ParseWrongLengthReturnsTransparent)
    {
        EXPECT_EQ(ParseColor("#FFF"), Color::Transparent);
    }

    TEST(LDtkColorParserTests, ParseNonHexReturnsTransparent)
    {
        EXPECT_EQ(ParseColor("#GGGGGG"), Color::Transparent);
    }

    TEST(LDtkColorParserTests, ParseTrailingGarbageReturnsTransparent)
    {
        // Regression check for the std::stoi-leniency bug class already fixed elsewhere this
        // session (XmlReaderExtensions.cpp): each 2-char byte must be entirely valid hex.
        EXPECT_EQ(ParseColor("#0g0000"), Color::Transparent);
    }

    // ParseColor always forces Alpha=255, so it can never actually produce Color::Transparent
    // (0,0,0,0) for any valid "#RRGGBB" input -- meaning upstream's own
    // `color != Color.Transparent || colorString == "#000000"` fallback clause is dead code; the
    // != check alone is already true for a successful parse. Ported faithfully anyway (see
    // LDtkColorParser.cpp), and this test exercises the real (opaque, not transparent) result.
    TEST(LDtkColorParserTests, TryParseColorBlackSucceedsWithOpaqueResult)
    {
        Color color(0, 0, 0, 0);
        EXPECT_TRUE(TryParseColor("#000000", color));
        EXPECT_EQ(color, Color(0, 0, 0, 255));
    }

    TEST(LDtkColorParserTests, TryParseColorInvalidFails)
    {
        Color color(0, 0, 0, 0);
        EXPECT_FALSE(TryParseColor("not-a-color", color));
    }

    // --- Document parsing ------------------------------------------------------------------------

    TEST(LDtkJsonParserTests, ParseProjectPopulatesTopLevelFields)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);

        EXPECT_EQ(project.Iid, "proj-iid");
        EXPECT_EQ(project.DefaultGridSize, 16);
        ASSERT_EQ(project.Levels.size(), 1u);
        ASSERT_TRUE(project.Defs.has_value());
        ASSERT_EQ(project.Defs->Tilesets.size(), 1u);
        EXPECT_EQ(project.Defs->Tilesets[0].Identifier, "Tileset");
    }

    TEST(LDtkJsonParserTests, ParseLevelPopulatesLayerInstances)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const Document::LDtkLevel& level = project.Levels[0];

        EXPECT_EQ(level.Identifier, "Level_0");
        ASSERT_TRUE(level.LayerInstances.has_value());
        EXPECT_EQ(level.LayerInstances->size(), 3u);
    }

    // --- Converters::Convert --------------------------------------------------------------------

    TEST(LDtkJsonParserTests, ConvertPopulatesTilemapDataBasics)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        EXPECT_EQ(data.Name, "Level_0");
        EXPECT_EQ(data.Orientation, TilemapOrientation::Orthogonal);
        EXPECT_EQ(data.TileWidth, 16);
        EXPECT_EQ(data.TileHeight, 16);
        ASSERT_EQ(data.Tilesets.size(), 1u);
        EXPECT_FALSE(data.Tilesets[0].IsExternal);
        ASSERT_TRUE(data.Tilesets[0].InlineData.has_value());
        EXPECT_EQ(data.Tilesets[0].InlineData->TexturePath, "tileset.png");
    }

    TEST(LDtkJsonParserTests, ConvertReversesLayerOrderTopToBottomBecomesBottomToTop)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        // Upstream JSON order (top-to-bottom): Entities, IntGrid, Tiles.
        // Runtime order (bottom-to-top, reversed): Tiles, IntGrid, Entities.
        ASSERT_EQ(data.Layers.size(), 3u);
        EXPECT_EQ(data.Layers[0]->Name, "Tiles");
        EXPECT_EQ(data.Layers[1]->Name, "IntGrid");
        EXPECT_EQ(data.Layers[2]->Name, "Entities");
    }

    TEST(LDtkJsonParserTests, ConvertTilesLayerDecodesGlobalIdsFromTilesetFirstGid)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        const TilemapTileLayerData* tiles = FindTileLayer(data, "Tiles");
        ASSERT_NE(tiles, nullptr);
        ASSERT_EQ(tiles->Tiles.size(), 2u);

        // Tiles land in different grid cells, so their relative order within `Tiles` depends on
        // the internal cell-grouping map's iteration order -- neither this port's
        // std::unordered_map nor upstream's own C# Dictionary<long, ...> make any real ordering
        // guarantee here (informal "insertion order" for small C# Dictionaries is a common but
        // undocumented implementation detail, not a contract this test should assume). Find each
        // decoded tile by its (X, Y) cell instead of asserting a specific array index.
        const auto findByCell = [&](std::uint16_t x, std::uint16_t y) -> const TilemapDecodedTile*
        {
            for (const TilemapDecodedTile& tile : tiles->Tiles)
            {
                if (tile.X == x && tile.Y == y)
                {
                    return &tile;
                }
            }
            return nullptr;
        };

        // Tileset uid=5 is the only one defined, so its first GID is 1: local tile id 0 -> gid 1,
        // local tile id 1 -> gid 2.
        const TilemapDecodedTile* cell00 = findByCell(0, 0);
        ASSERT_NE(cell00, nullptr);
        EXPECT_EQ(cell00->GlobalId, 1);
        EXPECT_EQ(cell00->FlipFlags, TilemapTileFlipFlags::None);

        const TilemapDecodedTile* cell10 = findByCell(1, 0);
        ASSERT_NE(cell10, nullptr);
        EXPECT_EQ(cell10->GlobalId, 2);
        EXPECT_EQ(cell10->FlipFlags, TilemapTileFlipFlags::FlipHorizontally);
    }

    TEST(LDtkJsonParserTests, ConvertIntGridLayerStoresCsvAsProperty)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        const TilemapDataLayerData* intGrid = FindDataLayer(data, "IntGrid");
        ASSERT_NE(intGrid, nullptr);

        const TilemapPropertyData* csv = FindProperty(intGrid->Properties, "LDtk_IntGridCsv");
        ASSERT_NE(csv, nullptr);
        EXPECT_EQ(csv->Type, TilemapPropertyType::String);
        EXPECT_EQ(csv->StringValue, "1,1,0,2");
    }

    TEST(LDtkJsonParserTests, ConvertEntityLayerCreatesRectangleObjectsWithTypedFields)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        const TilemapObjectLayerData* entities = FindObjectLayer(data, "Entities");
        ASSERT_NE(entities, nullptr);
        EXPECT_EQ(entities->DrawOrder, TilemapObjectDrawOrder::TopDown);
        ASSERT_EQ(entities->Objects.size(), 1u);

        const auto* rect = dynamic_cast<const TilemapRectangleObjectData*>(entities->Objects[0].get());
        ASSERT_NE(rect, nullptr);
        EXPECT_EQ(rect->Name, "Player");
        EXPECT_FLOAT_EQ(rect->X, 16.0f);
        EXPECT_FLOAT_EQ(rect->Y, 16.0f);
        EXPECT_FLOAT_EQ(rect->Width, 16.0f);
        EXPECT_FLOAT_EQ(rect->Height, 16.0f);
        EXPECT_EQ(rect->Class, "hero");

        const TilemapPropertyData* health = FindProperty(rect->Properties, "Health");
        ASSERT_NE(health, nullptr);
        EXPECT_EQ(health->Type, TilemapPropertyType::Int);
        EXPECT_EQ(health->IntValue, 100);

        const TilemapPropertyData* name = FindProperty(rect->Properties, "Name");
        ASSERT_NE(name, nullptr);
        EXPECT_EQ(name->Type, TilemapPropertyType::String);
        EXPECT_EQ(name->StringValue, "Hero");
    }

    TEST(LDtkJsonParserTests, ConvertLevelPropertiesIncludeWorldCoordinates)
    {
        const Document::LDtkProject project = ParseProject(kProjectJson);
        const TilemapData data = Converters::Convert(project.Levels[0], project);

        EXPECT_EQ(data.WorldX, 0);
        EXPECT_EQ(data.WorldY, 0);
        EXPECT_EQ(data.WorldDepth, 0);

        const TilemapPropertyData* iid = FindProperty(data.Properties, "LDtk_Iid");
        ASSERT_NE(iid, nullptr);
        EXPECT_EQ(iid->StringValue, "level-iid");
    }

    TEST(LDtkJsonParserTests, ConvertMissingLayerInstancesProducesNoLayers)
    {
        Document::LDtkLevel level;
        level.Identifier = "External";
        level.ExternalRelPath = "external.ldtkl";
        // LayerInstances left unset (nullopt), matching a level whose layers live in an external file.

        Document::LDtkProject project;
        const TilemapData data = Converters::Convert(level, project);

        EXPECT_EQ(data.Name, "External");
        EXPECT_TRUE(data.Layers.empty());
    }

    // --- LDtkJsonParser (headless-safe subset) ----------------------------------------------------

    TEST(LDtkJsonParserTests, ParserSupportedExtensionsContainsLdtk)
    {
        const LDtkJsonParser parser;
        ASSERT_EQ(parser.getSupportedExtensionsProperty().size(), 1u);
        EXPECT_EQ(parser.getSupportedExtensionsProperty()[0], ".ldtk");
    }

    TEST(LDtkJsonParserTests, CanParseWithInvalidExtensionReturnsFalse)
    {
        const LDtkJsonParser parser;
        EXPECT_FALSE(parser.CanParse("test.txt"));
    }

    TEST(LDtkJsonParserTests, CanParseWithEmptyPathReturnsFalse)
    {
        const LDtkJsonParser parser;
        EXPECT_FALSE(parser.CanParse(""));
    }

    TEST(LDtkJsonParserTests, CanParseWithLdtkExtensionReturnsTrueCaseInsensitive)
    {
        const LDtkJsonParser parser;
        EXPECT_TRUE(parser.CanParse("test.ldtk"));
        EXPECT_TRUE(parser.CanParse("test.LDTK"));
    }
}
