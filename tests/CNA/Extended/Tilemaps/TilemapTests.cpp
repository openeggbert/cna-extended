// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Tilemaps/TilemapTests.cs.
// Each upstream [Theory] becomes a TEST_P suite, matching this project's established
// parameterized-test convention.
#include "CNA/Extended/Tilemaps/Tilemap.hpp"

#include <gtest/gtest.h>
#include <tuple>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Point;

    // --- Orthogonal Coordinate Tests ---------------------------------------------------------

    class TileToWorldOrthogonalTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldOrthogonalTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 32, 32, TilemapOrientation::Orthogonal);
        const Point worldPos = tilemap.TileToWorldPosition(tileX, tileY);
        EXPECT_EQ(worldPos, Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldOrthogonalTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 0), std::make_tuple(1, 0, 32, 0), std::make_tuple(0, 1, 0, 32),
            std::make_tuple(1, 1, 32, 32), std::make_tuple(5, 3, 160, 96), std::make_tuple(10, 10, 320, 320)));

    class WorldToTileOrthogonalTests : public ::testing::TestWithParam<std::tuple<float, float, int, int>>
    {
    };
    TEST_P(WorldToTileOrthogonalTests, ReturnsExpectedCoordinates)
    {
        const auto [worldX, worldY, expectedTileX, expectedTileY] = GetParam();
        Tilemap tilemap("test", 20, 20, 32, 32, TilemapOrientation::Orthogonal);
        const Point tilePos = tilemap.WorldToTilePosition(Vector2(worldX, worldY));
        EXPECT_EQ(tilePos, Point(expectedTileX, expectedTileY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, WorldToTileOrthogonalTests,
        ::testing::Values(std::make_tuple(0.0f, 0.0f, 0, 0), std::make_tuple(32.0f, 0.0f, 1, 0), std::make_tuple(0.0f, 32.0f, 0, 1),
            std::make_tuple(32.0f, 32.0f, 1, 1), std::make_tuple(160.0f, 96.0f, 5, 3), std::make_tuple(320.0f, 320.0f, 10, 10)));

    TEST(TilemapTests, CoordinateTransformationOrthogonalRoundTripPreservesCoordinates)
    {
        Tilemap tilemap("test", 20, 20, 32, 32, TilemapOrientation::Orthogonal);
        const Point originalTile(5, 7);

        const Point worldPos = tilemap.TileToWorldPosition(originalTile.X, originalTile.Y);
        const Point backToTile = tilemap.WorldToTilePosition(Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)));

        EXPECT_EQ(backToTile, originalTile);
    }

    TEST(TilemapTests, WorldBoundsOrthogonalReturnsCorrectBounds)
    {
        Tilemap tilemap("test", 10, 8, 32, 32, TilemapOrientation::Orthogonal);
        const Rectangle bounds = tilemap.getWorldBoundsProperty();
        EXPECT_EQ(bounds, Rectangle(0, 0, 320, 256));
    }

    // --- Isometric Coordinate Tests -----------------------------------------------------------

    TEST(TilemapTests, TileToWorldPositionIsometricOriginReturnsZero)
    {
        Tilemap tilemap("test", 10, 10, 64, 32, TilemapOrientation::Isometric);
        EXPECT_EQ(tilemap.TileToWorldPosition(0, 0), Point::Zero);
    }

    class TileToWorldIsometricTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldIsometricTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 10, 10, 64, 32, TilemapOrientation::Isometric);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldIsometricTests,
        ::testing::Values(std::make_tuple(1, 0, 32, 16),   // (1-0)*(64/2), (1+0)*(32/2)
            std::make_tuple(0, 1, -32, 16),                // (0-1)*(64/2), (0+1)*(32/2)
            std::make_tuple(1, 1, 0, 32),                  // (1-1)*(64/2), (1+1)*(32/2)
            std::make_tuple(2, 1, 32, 48)));                // (2-1)*(64/2), (2+1)*(32/2)

    TEST(TilemapTests, WorldToTilePositionIsometricOriginReturnsZero)
    {
        Tilemap tilemap("test", 10, 10, 64, 32, TilemapOrientation::Isometric);
        EXPECT_EQ(tilemap.WorldToTilePosition(Vector2(0.0f, 0.0f)), Point::Zero);
    }

    TEST(TilemapTests, CoordinateTransformationIsometricRoundTripPreservesCoordinates)
    {
        Tilemap tilemap("test", 10, 10, 64, 32, TilemapOrientation::Isometric);
        const Point originalTile(5, 3);

        const Point worldPos = tilemap.TileToWorldPosition(originalTile.X, originalTile.Y);
        const Point backToTile = tilemap.WorldToTilePosition(Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)));

        EXPECT_EQ(backToTile, originalTile);
    }

    TEST(TilemapTests, WorldBoundsIsometricReturnsCorrectBounds)
    {
        Tilemap tilemap("test", 10, 8, 64, 32, TilemapOrientation::Isometric);
        const Rectangle bounds = tilemap.getWorldBoundsProperty();

        // (10 + 8) * (64/2) = 18 * 32 = 576 width
        // (10 + 8) * (32/2) = 18 * 16 = 288 height
        EXPECT_EQ(bounds, Rectangle(0, 0, 576, 288));
    }

    // --- Different Tile Sizes -----------------------------------------------------------------

    class TileToWorldOrthogonalDifferentTileSizesTests : public ::testing::TestWithParam<std::tuple<int, int>>
    {
    };
    TEST_P(TileToWorldOrthogonalDifferentTileSizesTests, WorksCorrectly)
    {
        const auto [tileWidth, tileHeight] = GetParam();
        Tilemap tilemap("test", 10, 10, tileWidth, tileHeight, TilemapOrientation::Orthogonal);
        const Point worldPos = tilemap.TileToWorldPosition(3, 2);
        EXPECT_EQ(worldPos, Point(3 * tileWidth, 2 * tileHeight));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldOrthogonalDifferentTileSizesTests,
        ::testing::Values(
            std::make_tuple(16, 16), std::make_tuple(32, 32), std::make_tuple(64, 64), std::make_tuple(48, 24)));

    // --- Edge Cases -----------------------------------------------------------------------

    TEST(TilemapTests, TileToWorldPositionNegativeCoordinatesHandlesCorrectly)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        EXPECT_EQ(tilemap.TileToWorldPosition(-1, -2), Point(-32, -64));
    }

    TEST(TilemapTests, WorldToTilePositionNegativeWorldPositionHandlesCorrectly)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        EXPECT_EQ(tilemap.WorldToTilePosition(Vector2(-32.0f, -64.0f)), Point(-1, -2));
    }

    TEST(TilemapTests, WorldToTilePositionFractionalWorldPositionTruncatesDown)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        const Point tilePos = tilemap.WorldToTilePosition(Vector2(50.7f, 70.9f));

        // 50/32 = 1.5 -> 1
        // 70/32 = 2.1 -> 2
        EXPECT_EQ(tilePos, Point(1, 2));
    }

    // --- Staggered Coordinate Tests ----------------------------------------------------------

    // Staggered Y-axis, Odd index: odd rows are offset right by TileWidth/2.
    class TileToWorldStaggeredYOddTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldStaggeredYOddTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 16, 16, TilemapOrientation::Staggered);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::Y);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Odd);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldStaggeredYOddTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 0), std::make_tuple(2, 0, 32, 0), std::make_tuple(0, 1, 8, 8),
            std::make_tuple(2, 1, 40, 8), std::make_tuple(0, 2, 0, 16), std::make_tuple(0, 3, 8, 24)));

    // Staggered Y-axis, Even index: even rows are offset right by TileWidth/2.
    class TileToWorldStaggeredYEvenTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldStaggeredYEvenTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 16, 16, TilemapOrientation::Staggered);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::Y);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Even);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldStaggeredYEvenTests,
        ::testing::Values(std::make_tuple(0, 0, 8, 0), std::make_tuple(2, 0, 40, 0), std::make_tuple(0, 1, 0, 8),
            std::make_tuple(2, 1, 32, 8), std::make_tuple(0, 2, 8, 16), std::make_tuple(0, 3, 0, 24)));

    // Staggered X-axis, Odd index: odd columns are offset down by TileHeight/2.
    class TileToWorldStaggeredXOddTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldStaggeredXOddTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 16, 16, TilemapOrientation::Staggered);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::X);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Odd);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldStaggeredXOddTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 2, 0, 32), std::make_tuple(1, 0, 8, 8),
            std::make_tuple(1, 2, 8, 40), std::make_tuple(2, 0, 16, 0), std::make_tuple(3, 0, 24, 8)));

    // Staggered X-axis, Even index: even columns are offset down by TileHeight/2.
    class TileToWorldStaggeredXEvenTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldStaggeredXEvenTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 16, 16, TilemapOrientation::Staggered);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::X);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Even);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldStaggeredXEvenTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 8), std::make_tuple(0, 2, 0, 40), std::make_tuple(1, 0, 8, 0),
            std::make_tuple(1, 2, 8, 32), std::make_tuple(2, 0, 16, 8), std::make_tuple(3, 0, 24, 0)));

    class CoordinateTransformationStaggeredTests
        : public ::testing::TestWithParam<std::tuple<TilemapStaggerAxis, TilemapStaggerIndex>>
    {
    };
    TEST_P(CoordinateTransformationStaggeredTests, RoundTripPreservesCoordinates)
    {
        const auto [axis, index] = GetParam();
        Tilemap tilemap("test", 20, 20, 16, 16, TilemapOrientation::Staggered);
        tilemap.setStaggerAxisProperty(axis);
        tilemap.setStaggerIndexProperty(index);
        const Point originalTile(5, 7);

        const Point worldPos = tilemap.TileToWorldPosition(originalTile.X, originalTile.Y);
        const Point backToTile = tilemap.WorldToTilePosition(Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)));

        EXPECT_EQ(backToTile, originalTile);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, CoordinateTransformationStaggeredTests,
        ::testing::Values(std::make_tuple(TilemapStaggerAxis::Y, TilemapStaggerIndex::Odd),
            std::make_tuple(TilemapStaggerAxis::Y, TilemapStaggerIndex::Even),
            std::make_tuple(TilemapStaggerAxis::X, TilemapStaggerIndex::Odd),
            std::make_tuple(TilemapStaggerAxis::X, TilemapStaggerIndex::Even)));

    // --- Hexagonal Coordinate Tests -----------------------------------------------------------

    // Hexagonal Y-axis, Odd index. Using 14x12 tiles with hexSideLength=6: rowStep=(12+6)/2=9, halfX=7.
    class TileToWorldHexagonalYOddTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldHexagonalYOddTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 14, 12, TilemapOrientation::Hexagonal);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::Y);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Odd);
        tilemap.setHexSideLengthProperty(6);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldHexagonalYOddTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 0), std::make_tuple(2, 0, 28, 0), std::make_tuple(0, 1, 7, 9),
            std::make_tuple(2, 1, 35, 9), std::make_tuple(0, 2, 0, 18), std::make_tuple(0, 3, 7, 27)));

    // Hexagonal Y-axis, Even index.
    class TileToWorldHexagonalYEvenTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldHexagonalYEvenTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 14, 12, TilemapOrientation::Hexagonal);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::Y);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Even);
        tilemap.setHexSideLengthProperty(6);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldHexagonalYEvenTests,
        ::testing::Values(std::make_tuple(0, 0, 7, 0), std::make_tuple(2, 0, 35, 0), std::make_tuple(0, 1, 0, 9),
            std::make_tuple(2, 1, 28, 9), std::make_tuple(0, 2, 7, 18), std::make_tuple(0, 3, 0, 27)));

    // Hexagonal X-axis, Odd index. Using 12x14 tiles with hexSideLength=6: colStep=(12+6)/2=9, halfY=7.
    class TileToWorldHexagonalXOddTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldHexagonalXOddTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 12, 14, TilemapOrientation::Hexagonal);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::X);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Odd);
        tilemap.setHexSideLengthProperty(6);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldHexagonalXOddTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 0), std::make_tuple(0, 2, 0, 28), std::make_tuple(1, 0, 9, 7),
            std::make_tuple(1, 2, 9, 35), std::make_tuple(2, 0, 18, 0), std::make_tuple(3, 0, 27, 7)));

    // Hexagonal X-axis, Even index.
    class TileToWorldHexagonalXEvenTests : public ::testing::TestWithParam<std::tuple<int, int, int, int>>
    {
    };
    TEST_P(TileToWorldHexagonalXEvenTests, ReturnsExpectedPositions)
    {
        const auto [tileX, tileY, expectedWorldX, expectedWorldY] = GetParam();
        Tilemap tilemap("test", 20, 20, 12, 14, TilemapOrientation::Hexagonal);
        tilemap.setStaggerAxisProperty(TilemapStaggerAxis::X);
        tilemap.setStaggerIndexProperty(TilemapStaggerIndex::Even);
        tilemap.setHexSideLengthProperty(6);
        EXPECT_EQ(tilemap.TileToWorldPosition(tileX, tileY), Point(expectedWorldX, expectedWorldY));
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, TileToWorldHexagonalXEvenTests,
        ::testing::Values(std::make_tuple(0, 0, 0, 7), std::make_tuple(0, 2, 0, 35), std::make_tuple(1, 0, 9, 0),
            std::make_tuple(1, 2, 9, 28), std::make_tuple(2, 0, 18, 7), std::make_tuple(3, 0, 27, 0)));

    class CoordinateTransformationHexagonalTests
        : public ::testing::TestWithParam<std::tuple<TilemapStaggerAxis, TilemapStaggerIndex>>
    {
    };
    TEST_P(CoordinateTransformationHexagonalTests, RoundTripPreservesCoordinates)
    {
        const auto [axis, index] = GetParam();

        // Use asymmetric tile sizes to verify both axes are handled independently.
        const int tileWidth = axis == TilemapStaggerAxis::Y ? 14 : 12;
        const int tileHeight = axis == TilemapStaggerAxis::Y ? 12 : 14;

        Tilemap tilemap("test", 20, 20, tileWidth, tileHeight, TilemapOrientation::Hexagonal);
        tilemap.setStaggerAxisProperty(axis);
        tilemap.setStaggerIndexProperty(index);
        tilemap.setHexSideLengthProperty(6);
        const Point originalTile(5, 7);

        const Point worldPos = tilemap.TileToWorldPosition(originalTile.X, originalTile.Y);
        const Point backToTile = tilemap.WorldToTilePosition(Vector2(static_cast<float>(worldPos.X), static_cast<float>(worldPos.Y)));

        EXPECT_EQ(backToTile, originalTile);
    }
    INSTANTIATE_TEST_SUITE_P(TilemapTests, CoordinateTransformationHexagonalTests,
        ::testing::Values(std::make_tuple(TilemapStaggerAxis::Y, TilemapStaggerIndex::Odd),
            std::make_tuple(TilemapStaggerAxis::Y, TilemapStaggerIndex::Even),
            std::make_tuple(TilemapStaggerAxis::X, TilemapStaggerIndex::Odd),
            std::make_tuple(TilemapStaggerAxis::X, TilemapStaggerIndex::Even)));
}
