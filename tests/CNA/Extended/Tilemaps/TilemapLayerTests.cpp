// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Tilemaps/TilemapLayers/TilemapLayerTests.cs. Upstream keeps a
// reference to a layer both before and after `Layers.Add(layer)` for identity comparison; since
// `TilemapLayerCollection::Add` takes ownership (`std::unique_ptr<TilemapLayer>`), each test
// below captures the raw pointer before moving the owning pointer into `Add`.
#include "CNA/Extended/Tilemaps/Tilemap.hpp"
#include "CNA/Extended/Tilemaps/TilemapObjectLayer.hpp"
#include "CNA/Extended/Tilemaps/TilemapTileLayer.hpp"

#include <algorithm>
#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps
{
    TEST(TilemapLayerTests, GetLayerWithExistingLayerReturnsLayer)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        auto layer = std::make_unique<TilemapTileLayer>("TestLayer", 10, 10, 32, 32);
        TilemapTileLayer* raw = layer.get();
        tilemap.getLayersProperty().Add(std::move(layer));

        EXPECT_EQ(tilemap.getLayersProperty()["TestLayer"], raw);
    }

    TEST(TilemapLayerTests, GetLayerWithNonExistingLayerReturnsNull)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        TilemapLayer* result = nullptr;
        const bool hasLayer = tilemap.getLayersProperty().TryGetValue("NonExistent", result);
        EXPECT_FALSE(hasLayer);
        EXPECT_EQ(result, nullptr);
    }

    TEST(TilemapLayerTests, GetLayerGenericWithCorrectTypeReturnsLayer)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        auto tileLayer = std::make_unique<TilemapTileLayer>("TileLayer", 10, 10, 32, 32);
        TilemapTileLayer* raw = tileLayer.get();
        tilemap.getLayersProperty().Add(std::move(tileLayer));

        TilemapTileLayer* result = tilemap.getLayersProperty().GetLayer<TilemapTileLayer>("TileLayer");

        EXPECT_EQ(result, raw);
    }

    TEST(TilemapLayerTests, GetLayerGenericWithWrongTypeReturnsNull)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        auto tileLayer = std::make_unique<TilemapTileLayer>("TileLayer", 10, 10, 32, 32);
        tilemap.getLayersProperty().Add(std::move(tileLayer));

        TilemapObjectLayer* result = tilemap.getLayersProperty().GetLayer<TilemapObjectLayer>("TileLayer");

        EXPECT_EQ(result, nullptr);
    }

    TEST(TilemapLayerTests, GetLayersGenericReturnsLayersOfType)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        auto tileLayer1 = std::make_unique<TilemapTileLayer>("Tiles1", 10, 10, 32, 32);
        auto tileLayer2 = std::make_unique<TilemapTileLayer>("Tiles2", 10, 10, 32, 32);
        auto objectLayer = std::make_unique<TilemapObjectLayer>("Objects");
        TilemapTileLayer* raw1 = tileLayer1.get();
        TilemapTileLayer* raw2 = tileLayer2.get();

        tilemap.getLayersProperty().Add(std::move(tileLayer1));
        tilemap.getLayersProperty().Add(std::move(objectLayer));
        tilemap.getLayersProperty().Add(std::move(tileLayer2));

        const std::vector<TilemapTileLayer*> tileLayers = tilemap.getLayersProperty().GetLayers<TilemapTileLayer>();

        ASSERT_EQ(tileLayers.size(), 2u);
        EXPECT_NE(std::find(tileLayers.begin(), tileLayers.end(), raw1), tileLayers.end());
        EXPECT_NE(std::find(tileLayers.begin(), tileLayers.end(), raw2), tileLayers.end());
    }

    TEST(TilemapLayerTests, GetLayersGenericWithNoMatchingLayersReturnsEmpty)
    {
        Tilemap tilemap("test", 10, 10, 32, 32, TilemapOrientation::Orthogonal);
        auto tileLayer = std::make_unique<TilemapTileLayer>("Tiles", 10, 10, 32, 32);
        tilemap.getLayersProperty().Add(std::move(tileLayer));

        const std::vector<TilemapObjectLayer*> objectLayers = tilemap.getLayersProperty().GetLayers<TilemapObjectLayer>();

        EXPECT_TRUE(objectLayers.empty());
    }
}
