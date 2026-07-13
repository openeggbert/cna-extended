// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tilemaps/TilemapTilesetTests.cs.
// Upstream uses a `GraphicsTestFixture.CreatePixelTexture()` requiring a live GraphicsDevice;
// substituted with `Texture2D::CreateCpuOnlyForTests` (no GraphicsDevice needed), matching this
// project's established pattern for headless texture-backed tests elsewhere in this module.
#include "CNA/Extended/Tilemaps/TilemapTileset.hpp"

#include "CNA/Extended/Tilemaps/TilemapTileData.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/InvalidOperationException.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    namespace
    {
        Texture2D CreatePixelTexture()
        {
            return Texture2D::CreateCpuOnlyForTests(1, 1, SurfaceFormat::Color, std::vector<Color>{Color::White});
        }
    }

    class TilemapTilesetGetSourceRectangleTests : public ::testing::TestWithParam<std::tuple<int, int, int>>
    {
    };
    TEST_P(TilemapTilesetGetSourceRectangleTests, CalculatesCorrectPosition)
    {
        const auto [localId, expectedX, expectedY] = GetParam();
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 32, 32, 100, 10);

        const Rectangle rect = tileset.GetTileRegion(localId);

        EXPECT_EQ(rect.X, expectedX);
        EXPECT_EQ(rect.Y, expectedY);
        EXPECT_EQ(rect.Width, 32);
        EXPECT_EQ(rect.Height, 32);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTilesetTests, TilemapTilesetGetSourceRectangleTests,
        ::testing::Values(std::make_tuple(0, 0, 0),     // First tile
            std::make_tuple(1, 32, 0),                  // Second tile (column 1)
            std::make_tuple(9, 288, 0),                 // Last tile in first row (column 9)
            std::make_tuple(10, 0, 32),                 // First tile in second row
            std::make_tuple(11, 32, 32),                // Second tile in second row
            std::make_tuple(25, 160, 64)));              // Tile at column 5, row 2

    TEST(TilemapTilesetTests, GetSourceRectangleWithDifferentTileSizeCalculatesCorrectly)
    {
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 16, 24, 100, 8);

        // Column 2, Row 1
        const Rectangle rect = tileset.GetTileRegion(10);

        EXPECT_EQ(rect.X, 32);
        EXPECT_EQ(rect.Y, 24);
        EXPECT_EQ(rect.Width, 16);
        EXPECT_EQ(rect.Height, 24);
    }

    TEST(TilemapTilesetTests, GetTileDataWithNoDataReturnsNull)
    {
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 32, 32, 100, 10);

        EXPECT_EQ(tileset.GetTileData(5), nullptr);
    }

    TEST(TilemapTilesetTests, AddTileDataAndGetTileDataReturnsData)
    {
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 32, 32, 100, 10);
        auto tileData = std::make_unique<TilemapTileData>(5);
        tileData->setClassProperty("Water");

        tileset.AddTileData(std::move(tileData));
        TilemapTileData* retrieved = tileset.GetTileData(5);

        ASSERT_NE(retrieved, nullptr);
        EXPECT_EQ(retrieved->getLocalIdProperty(), 5);
        EXPECT_EQ(retrieved->getClassProperty(), "Water");
    }

    TEST(TilemapTilesetTests, AddTileDataWithMultipleTilesCanRetrieveEach)
    {
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 32, 32, 100, 10);
        auto data1 = std::make_unique<TilemapTileData>(1);
        data1->setClassProperty("Grass");
        auto data2 = std::make_unique<TilemapTileData>(2);
        data2->setClassProperty("Stone");
        auto data3 = std::make_unique<TilemapTileData>(3);
        data3->setClassProperty("Water");

        tileset.AddTileData(std::move(data1));
        tileset.AddTileData(std::move(data2));
        tileset.AddTileData(std::move(data3));

        ASSERT_NE(tileset.GetTileData(1), nullptr);
        EXPECT_EQ(tileset.GetTileData(1)->getClassProperty(), "Grass");
        ASSERT_NE(tileset.GetTileData(2), nullptr);
        EXPECT_EQ(tileset.GetTileData(2)->getClassProperty(), "Stone");
        ASSERT_NE(tileset.GetTileData(3), nullptr);
        EXPECT_EQ(tileset.GetTileData(3)->getClassProperty(), "Water");
        EXPECT_EQ(tileset.GetTileData(4), nullptr);
    }

    TEST(TilemapTilesetTests, AddTileDataWithSameLocalIdOverwritesPrevious)
    {
        Texture2D texture = CreatePixelTexture();
        TilemapTileset tileset("Test", &texture, 32, 32, 100, 10);
        auto data1 = std::make_unique<TilemapTileData>(5);
        data1->setClassProperty("Old");
        auto data2 = std::make_unique<TilemapTileData>(5);
        data2->setClassProperty("New");

        tileset.AddTileData(std::move(data1));
        tileset.AddTileData(std::move(data2));
        TilemapTileData* retrieved = tileset.GetTileData(5);

        ASSERT_NE(retrieved, nullptr);
        EXPECT_EQ(retrieved->getClassProperty(), "New");
    }

    TEST(TilemapTilesetTests, GetTileRegionOnCollectionTilesetThrowsInvalidOperationException)
    {
        // Regression test: collection tilesets have Columns=0, which caused a
        // DivideByZeroException upstream. This port's translation of that division uses
        // C++ int division, which would instead trigger UB/SIGFPE on a literal zero divisor --
        // GetTileRegion's own Columns==0 guard (already present, matching upstream's explicit
        // check) is what prevents that here too.
        TilemapTileset tileset("Collection", nullptr, 128, 108, 7, 0);

        EXPECT_THROW((void)tileset.GetTileRegion(0), System::InvalidOperationException);
    }
}
