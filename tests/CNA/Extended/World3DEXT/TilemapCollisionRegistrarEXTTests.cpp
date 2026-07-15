// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
//
// New, non-upstream tests for TilemapCollisionActor3DEXT/RegisterTilemapCollisionActorsEXT
// (see 3d.md/plan3d.md) -- there is no upstream MonoGame.Extended test suite to port here.
#include "CNA/Extended/World3DEXT/TilemapCollisionRegistrarEXT.hpp"

#include "CNA/Extended/World3DEXT/CollisionShape3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Layer3DEXT.hpp"
#include "CNA/Extended/World3DEXT/SpatialHash3DEXT.hpp"
#include "CNA/Extended/World3DEXT/Tilemap3DEXT.hpp"
#include "CNA/Extended/World3DEXT/TilemapCollisionActor3DEXT.hpp"
#include "Microsoft/Xna/Framework/BoundingBox.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::World3DEXT
{
    using Microsoft::Xna::Framework::BoundingBox;
    using Microsoft::Xna::Framework::Vector3;

    TEST(TilemapCollisionActor3DEXTTests, ExposesTileCoordinateAndId)
    {
        const CollisionShape3DEXT shape(BoundingBox(Vector3::Zero, Vector3::One));
        TilemapCollisionActor3DEXT actor(7, 1, 2, 3, 42, shape);

        EXPECT_EQ(actor.getIdProperty(), 7);
        EXPECT_EQ(actor.getTileXEXTProperty(), 1);
        EXPECT_EQ(actor.getTileYEXTProperty(), 2);
        EXPECT_EQ(actor.getTileZEXTProperty(), 3);
        EXPECT_EQ(actor.getTileIdEXTProperty(), 42);
    }

    TEST(TilemapCollisionRegistrarEXTTests, RegistersOneActorPerPopulatedTile)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);
        tilemap.SetTileEXT(1, 0, 0, 2);
        tilemap.SetTileEXT(0, 1, 0, 3);

        CollisionWorld3DEXT world;
        const std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors = RegisterTilemapCollisionActorsEXT(world, tilemap);

        EXPECT_EQ(actors.size(), 3u);
        for (const std::unique_ptr<TilemapCollisionActor3DEXT>& actor : actors)
        {
            EXPECT_TRUE(world.Contains(actor.get()));
        }
    }

    TEST(TilemapCollisionRegistrarEXTTests, EmptyTilemap_RegistersNoActors)
    {
        Tilemap3DEXT tilemap;
        CollisionWorld3DEXT world;

        const std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors = RegisterTilemapCollisionActorsEXT(world, tilemap);

        EXPECT_TRUE(actors.empty());
    }

    TEST(TilemapCollisionRegistrarEXTTests, ActorShapeMatchesTileWorldBounds)
    {
        Tilemap3DEXT tilemap(Vector3(2.0f, 2.0f, 2.0f));
        tilemap.SetTileEXT(1, 0, 0, 5);

        CollisionWorld3DEXT world;
        const std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors = RegisterTilemapCollisionActorsEXT(world, tilemap);

        ASSERT_EQ(actors.size(), 1u);
        const BoundingBox box = actors[0]->getShapeProperty().getBoundingBoxProperty();
        const Vector3 expectedCenter = tilemap.TileToWorldPositionEXT(1, 0, 0);
        EXPECT_NEAR(box.Min.X, expectedCenter.X - 1.0f, 1e-4f);
        EXPECT_NEAR(box.Max.X, expectedCenter.X + 1.0f, 1e-4f);
    }

    TEST(TilemapCollisionRegistrarEXTTests, RegistersIntoNamedLayer)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);

        CollisionWorld3DEXT world;
        world.AddLayer("terrain", std::make_unique<Layer3DEXT>(std::make_unique<SpatialHash3DEXT>(16.0f)));

        const std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors = RegisterTilemapCollisionActorsEXT(world, tilemap, "terrain");

        ASSERT_EQ(actors.size(), 1u);
        std::string layerName;
        ASSERT_TRUE(world.TryGetLayerName(actors[0].get(), layerName));
        EXPECT_EQ(layerName, "terrain");
    }

    TEST(TilemapCollisionRegistrarEXTTests, QueryFindsRegisteredTileActor)
    {
        Tilemap3DEXT tilemap;
        tilemap.SetTileEXT(0, 0, 0, 1);

        CollisionWorld3DEXT world;
        const std::vector<std::unique_ptr<TilemapCollisionActor3DEXT>> actors = RegisterTilemapCollisionActorsEXT(world, tilemap);
        ASSERT_EQ(actors.size(), 1u);

        const BoundingBox queryBounds(Vector3(-5.0f, -5.0f, -5.0f), Vector3(5.0f, 5.0f, 5.0f));
        const std::vector<ICollisionActor3DEXT*> candidates = world.QueryCandidates(queryBounds);

        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0], actors[0].get());
    }
}
