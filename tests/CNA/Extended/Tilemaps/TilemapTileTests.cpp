// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tilemaps/TilemapTileTests.cs.
// Upstream's `CreateDummyTexture()` returns null ("TODO: setup with xvfb later") -- matched
// exactly (`nullptr`), not upgraded to `Texture2D::CreateCpuOnlyForTests`, since these tests
// don't exercise texture-dependent behavior at all.
#include "CNA/Extended/Tilemaps/TilemapTile.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"
#include "CNA/Extended/Tilemaps/TilemapTilesetCollection.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    namespace
    {
        TilemapTilesetCollection CreateTestTilesets()
        {
            TilemapTilesetCollection collection;

            auto tileset1 = std::make_unique<TilemapTileset>("Tileset1", nullptr, 32, 32, 100, 10);
            tileset1->setFirstGlobalIdProperty(1);

            auto tileset2 = std::make_unique<TilemapTileset>("Tileset2", nullptr, 32, 32, 50, 5);
            tileset2->setFirstGlobalIdProperty(101);

            collection.Add(std::move(tileset1));
            collection.Add(std::move(tileset2));

            return collection;
        }
    }

    TEST(TilemapTileTests, GetTilesetWithValidGidReturnsTileset)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        const TilemapTile tile(50);

        TilemapTileset* tileset = tile.GetTileset(tilesets);

        ASSERT_NE(tileset, nullptr);
        EXPECT_EQ(tileset->getNameProperty(), "Tileset1");
    }

    TEST(TilemapTileTests, GetTilesetWithInvalidGidReturnsNull)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        const TilemapTile tile(999);

        EXPECT_EQ(tile.GetTileset(tilesets), nullptr);
    }

    TEST(TilemapTileTests, GetLocalIdWithOutParameterReturnsTilesetAndLocalId)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        const TilemapTile tile(125);

        TilemapTileset* tileset = nullptr;
        const int localId = tile.GetLocalId(tilesets, tileset);

        EXPECT_EQ(localId, 24);
        ASSERT_NE(tileset, nullptr);
        EXPECT_EQ(tileset->getNameProperty(), "Tileset2");
    }

    TEST(TilemapTileTests, GetTileDataWithValidGidReturnsTileData)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        auto tileData = std::make_unique<TilemapTileData>(49);
        tileData->setClassProperty("Water");
        tilesets[0]->AddTileData(std::move(tileData));

        // Tileset1 FirstGlobalId=1, so localId=49
        const TilemapTile tile(50);

        const TilemapTileData* retrieved = tile.GetTileData(tilesets);

        ASSERT_NE(retrieved, nullptr);
        EXPECT_EQ(retrieved->getClassProperty(), "Water");
    }

    TEST(TilemapTileTests, GetTileDataWithNoTileDataReturnsNull)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        const TilemapTile tile(50);

        EXPECT_EQ(tile.GetTileData(tilesets), nullptr);
    }

    TEST(TilemapTileTests, GetTileDataWithInvalidGidReturnsNull)
    {
        TilemapTilesetCollection tilesets = CreateTestTilesets();
        const TilemapTile tile(999);

        EXPECT_EQ(tile.GetTileData(tilesets), nullptr);
    }
}
