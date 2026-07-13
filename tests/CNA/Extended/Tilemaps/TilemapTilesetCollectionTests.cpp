// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Tilemaps/TilemapTilesetCollectionTests.cs. Upstream's
// `CreateDummyTexture()` returns null ("TODO: setup with xvfb later") -- matched exactly
// (`nullptr`), since these tests exercise gid/local-id lookup logic, not texture-dependent
// behavior. Since `TilemapTilesetCollection::Add` takes ownership, each test captures the raw
// pointer before moving the owning pointer into `Add`.
#include "CNA/Extended/Tilemaps/TilemapTilesetCollection.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithSingleTilesetReturnsCorrectTileset)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(1);
        TilemapTileset* raw = tileset.get();
        collection.Add(std::move(tileset));

        EXPECT_EQ(collection.GetTilesetForGid(50), raw);
    }

    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithMultipleTilesetsReturnsCorrectTileset)
    {
        TilemapTilesetCollection collection;

        auto tileset1 = std::make_unique<TilemapTileset>("Tileset1", nullptr, 32, 32, 100, 10);
        tileset1->setFirstGlobalIdProperty(1);
        TilemapTileset* raw1 = tileset1.get();

        // After tileset1
        auto tileset2 = std::make_unique<TilemapTileset>("Tileset2", nullptr, 32, 32, 50, 5);
        tileset2->setFirstGlobalIdProperty(101);
        TilemapTileset* raw2 = tileset2.get();

        // After tileset2
        auto tileset3 = std::make_unique<TilemapTileset>("Tileset3", nullptr, 32, 32, 25, 5);
        tileset3->setFirstGlobalIdProperty(151);
        TilemapTileset* raw3 = tileset3.get();

        collection.Add(std::move(tileset1));
        collection.Add(std::move(tileset2));
        collection.Add(std::move(tileset3));

        // First tile of tileset1
        EXPECT_EQ(collection.GetTilesetForGid(1), raw1);
        // Mid tileset1
        EXPECT_EQ(collection.GetTilesetForGid(50), raw1);
        // Last tile of tileset1
        EXPECT_EQ(collection.GetTilesetForGid(100), raw1);

        // First tile of tileset2
        EXPECT_EQ(collection.GetTilesetForGid(101), raw2);
        // Mid of tileset2
        EXPECT_EQ(collection.GetTilesetForGid(125), raw2);
        // Last tile of tileset2
        EXPECT_EQ(collection.GetTilesetForGid(150), raw2);

        // First tile of tileset3
        EXPECT_EQ(collection.GetTilesetForGid(151), raw3);
        // Last tile of tileset3
        EXPECT_EQ(collection.GetTilesetForGid(175), raw3);
    }

    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithGidBeforeFirstTilesetReturnsNull)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(10);
        collection.Add(std::move(tileset));

        EXPECT_EQ(collection.GetTilesetForGid(5), nullptr);
    }

    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithEmptyCollectionReturnsNull)
    {
        TilemapTilesetCollection collection;
        EXPECT_EQ(collection.GetTilesetForGid(1), nullptr);
    }

    TEST(TilemapTilesetCollectionTests, GetLocalIdWithValidGidReturnsCorrectLocalId)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(1);
        TilemapTileset* raw = tileset.get();
        collection.Add(std::move(tileset));

        TilemapTileset* outTileset = nullptr;
        const int localId = collection.GetLocalId(50, outTileset);

        // 50 - 1 = 49
        EXPECT_EQ(localId, 49);
        EXPECT_EQ(outTileset, raw);
    }

    TEST(TilemapTilesetCollectionTests, GetLocalIdWithFirstGidReturnsZero)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(100);
        TilemapTileset* raw = tileset.get();
        collection.Add(std::move(tileset));

        TilemapTileset* outTileset = nullptr;
        const int localId = collection.GetLocalId(100, outTileset);

        EXPECT_EQ(localId, 0);
        EXPECT_EQ(outTileset, raw);
    }

    TEST(TilemapTilesetCollectionTests, GetLocalIdWithMultipleTilesetsReturnsCorrectLocalId)
    {
        TilemapTilesetCollection collection;

        auto tileset1 = std::make_unique<TilemapTileset>("Tileset1", nullptr, 32, 32, 100, 10);
        tileset1->setFirstGlobalIdProperty(1);
        TilemapTileset* raw1 = tileset1.get();

        auto tileset2 = std::make_unique<TilemapTileset>("Tileset2", nullptr, 32, 32, 50, 5);
        tileset2->setFirstGlobalIdProperty(101);
        TilemapTileset* raw2 = tileset2.get();

        collection.Add(std::move(tileset1));
        collection.Add(std::move(tileset2));

        TilemapTileset* outTileset1 = nullptr;
        const int localId1 = collection.GetLocalId(50, outTileset1);
        TilemapTileset* outTileset2 = nullptr;
        const int localId2 = collection.GetLocalId(125, outTileset2);

        EXPECT_EQ(localId1, 49);
        EXPECT_EQ(outTileset1, raw1);

        EXPECT_EQ(localId2, 24);
        EXPECT_EQ(outTileset2, raw2);
    }

    TEST(TilemapTilesetCollectionTests, GetLocalIdWithInvalidGidThrowsInvalidOperationException)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(10);
        collection.Add(std::move(tileset));

        TilemapTileset* outTileset = nullptr;
        EXPECT_THROW((void)collection.GetLocalId(5, outTileset), System::InvalidOperationException);
    }

    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithCollectionTilesetNonSequentialIdsReturnsCorrectTileset)
    {
        // Regression test: collection tilesets assign arbitrary tile IDs that can be much larger
        // than tileCount, so the lookup must use the tracked max local ID, not tileCount. The
        // data for this test mirrors a real world case that was given by a user on discord.
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Props", nullptr, 56, 63, 35, 0);
        tileset->setFirstGlobalIdProperty(6106);
        tileset->AddTileData(std::make_unique<TilemapTileData>(86));
        tileset->AddTileData(std::make_unique<TilemapTileData>(87));
        tileset->AddTileData(std::make_unique<TilemapTileData>(187));
        TilemapTileset* raw = tileset.get();
        collection.Add(std::move(tileset));

        // GlobalId 6192 = firstGid(6106) + localId(86)
        TilemapTileset* foundTileset = collection.GetTilesetForGid(6192);
        TilemapTileset* outTileset = nullptr;
        const int localId = collection.GetLocalId(6192, outTileset);

        EXPECT_EQ(foundTileset, raw);
        EXPECT_EQ(localId, 86);
        EXPECT_EQ(outTileset, raw);
    }

    // Regression tests for issue #1157: empty Tiled gid 0 must be treated as the empty no tile.
    TEST(TilemapTilesetCollectionTests, GetTilesetForGidWithEmptyTileGidReturnsNull)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(1);
        collection.Add(std::move(tileset));

        EXPECT_EQ(collection.GetTilesetForGid(0), nullptr);
    }

    TEST(TilemapTilesetCollectionTests, GetLocalIdWithEmptyTileGidReturnsZeroAndNullTileset)
    {
        TilemapTilesetCollection collection;
        auto tileset = std::make_unique<TilemapTileset>("Tileset", nullptr, 32, 32, 100, 10);
        tileset->setFirstGlobalIdProperty(1);
        collection.Add(std::move(tileset));

        TilemapTileset* outTileset = nullptr;
        const int localId = collection.GetLocalId(0, outTileset);

        EXPECT_EQ(localId, 0);
        EXPECT_EQ(outTileset, nullptr);
    }
}
