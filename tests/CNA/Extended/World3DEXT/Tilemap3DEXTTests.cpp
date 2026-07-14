// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for Tilemap3DEXT/Tilemap3DFactoryEXT/TilemapTileset3DEXT (see
// 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"

#include "CNA/Extended/World3DEXT/Tilemap3DFactoryEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapTileset3DEXT.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    TEST(Tilemap3DEXTTests, DefaultTile_IsEmpty)
    {
        Tilemap3DEXT tilemap;
        EXPECT_EQ(tilemap.GetTileEXT(0, 0, 0), 0);
        EXPECT_FALSE(tilemap.HasTileEXT(0, 0, 0));
    }

    TEST(Tilemap3DEXTTests, SetTileEXT_ThenGetTileEXT_ReturnsSameId)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(1, 2, 3, 42);

        EXPECT_EQ(tilemap.GetTileEXT(1, 2, 3), 42);
        EXPECT_TRUE(tilemap.HasTileEXT(1, 2, 3));
        EXPECT_EQ(tilemap.GetTileCountEXT(), 1u);
    }

    TEST(Tilemap3DEXTTests, SetTileEXT_ToZero_RemovesTile)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 5);
        tilemap.SetTileEXT(0, 0, 0, 0);

        EXPECT_FALSE(tilemap.HasTileEXT(0, 0, 0));
        EXPECT_EQ(tilemap.GetTileCountEXT(), 0u);
    }

    TEST(Tilemap3DEXTTests, RemoveTileEXT_ClearsTile)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 5);
        tilemap.RemoveTileEXT(0, 0, 0);

        EXPECT_FALSE(tilemap.HasTileEXT(0, 0, 0));
    }

    TEST(Tilemap3DEXTTests, TileToWorldPositionEXT_ScalesByTileSize)
    {
        Tilemap3DEXT tilemap(Vector3(2.0f, 3.0f, 4.0f));
        EXPECT_EQ(tilemap.TileToWorldPositionEXT(2, 3, 1), Vector3(4.0f, 9.0f, 4.0f));
    }

    TEST(Tilemap3DEXTTests, GetTilesProperty_ReturnsOnlyNonEmptyTiles)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(1, 0, 0, 2);
        tilemap.SetTileEXT(1, 0, 0, 0); // set then cleared, should not appear

        EXPECT_EQ(tilemap.getTilesProperty().size(), 1u);
        EXPECT_EQ(tilemap.getTilesProperty().at(TileCoordinate3DEXT(0, 0, 0)), 1);
    }

    TEST(Tilemap3DFactoryEXTTests, BuildFromArrayEXT_PopulatesNonZeroTilesOnly)
    {
        // 2x1x2 grid, row-major X fastest then Y then Z: index = (z*height + y)*width + x
        const std::vector<int> tileIds = {1, 0, 0, 2};
        const Tilemap3DEXT tilemap = Tilemap3DFactoryEXT::BuildFromArrayEXT(tileIds, 2, 1, 2, Vector3(1.0f, 1.0f, 1.0f));

        EXPECT_EQ(tilemap.GetTileEXT(0, 0, 0), 1);
        EXPECT_EQ(tilemap.GetTileEXT(1, 0, 0), 0);
        EXPECT_EQ(tilemap.GetTileEXT(0, 0, 1), 0);
        EXPECT_EQ(tilemap.GetTileEXT(1, 0, 1), 2);
        EXPECT_EQ(tilemap.GetTileCountEXT(), 2u);
    }

    TEST(Tilemap3DFactoryEXTTests, BuildFromArrayEXT_MismatchedSize_Throws)
    {
        const std::vector<int> tileIds = {1, 2, 3};
        EXPECT_THROW((void)Tilemap3DFactoryEXT::BuildFromArrayEXT(tileIds, 2, 2, 2, Vector3(1.0f, 1.0f, 1.0f)), std::exception);
    }

    TEST(TilemapTileset3DEXTTests, GetTileTextureEXT_WhenUnset_ReturnsNull)
    {
        TilemapTileset3DEXT tileset;
        EXPECT_EQ(tileset.GetTileTextureEXT(1), nullptr);
    }

    TEST(TilemapTileset3DEXTTests, SetTileTextureEXT_ThenGetTileTextureEXT_ReturnsSameTexture)
    {
        GraphicsDevice graphicsDevice;
        Texture2D texture(graphicsDevice, 1, 1);

        TilemapTileset3DEXT tileset;
        tileset.SetTileTextureEXT(7, &texture);

        EXPECT_EQ(tileset.GetTileTextureEXT(7), &texture);
    }
}
