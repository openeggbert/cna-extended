// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for `TilemapRendererShared` (it's an
// `internal static class` with no [InternalsVisibleTo]-based test access). These are fresh
// tests covering the pure math (`CalculateTextureCoordinates`, `AddTileQuad`) and the
// GraphicsDevice-adjacent construction paths (`CreateLayerModel`, `GetWrapSamplerState`) --
// all headless, using the same `GraphicsDevice`/`Texture2D` construction pattern established in
// OrthographicCameraTests.cpp.
#include "CNA/Extended/Tilemaps/Rendering/TilemapRendererShared.hpp"

#include "Microsoft/Xna/Framework/Vector3.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Tilemaps::Rendering
{
    using Microsoft::Xna::Framework::Vector3;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;

    namespace
    {
        class TilemapRendererSharedTest : public ::testing::Test
        {
        protected:
            GraphicsDevice graphicsDevice;
        };
    }

    TEST(CalculateTextureCoordinatesTest, NoFlip_MapsCornersDirectly)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 100, 100);
        const Rectangle sourceRect(25, 50, 25, 25);

        const std::array<Vector2, 4> uvs = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::None, texture);

        // top-left, top-right, bottom-left, bottom-right.
        EXPECT_FLOAT_EQ(uvs[0].X, 0.25f);
        EXPECT_FLOAT_EQ(uvs[0].Y, 0.50f);
        EXPECT_FLOAT_EQ(uvs[1].X, 0.50f);
        EXPECT_FLOAT_EQ(uvs[1].Y, 0.50f);
        EXPECT_FLOAT_EQ(uvs[2].X, 0.25f);
        EXPECT_FLOAT_EQ(uvs[2].Y, 0.75f);
        EXPECT_FLOAT_EQ(uvs[3].X, 0.50f);
        EXPECT_FLOAT_EQ(uvs[3].Y, 0.75f);
    }

    TEST(CalculateTextureCoordinatesTest, FlipHorizontally_SwapsLeftAndRightColumns)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 100, 100);
        const Rectangle sourceRect(0, 0, 50, 50);

        const std::array<Vector2, 4> noFlip = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::None, texture);
        const std::array<Vector2, 4> flipped = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::FlipHorizontally, texture);

        EXPECT_FLOAT_EQ(flipped[0].X, noFlip[1].X);
        EXPECT_FLOAT_EQ(flipped[1].X, noFlip[0].X);
        EXPECT_FLOAT_EQ(flipped[0].Y, noFlip[0].Y);
        EXPECT_FLOAT_EQ(flipped[2].X, noFlip[3].X);
        EXPECT_FLOAT_EQ(flipped[3].X, noFlip[2].X);
    }

    TEST(CalculateTextureCoordinatesTest, FlipVertically_SwapsTopAndBottomRows)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 100, 100);
        const Rectangle sourceRect(0, 0, 50, 50);

        const std::array<Vector2, 4> noFlip = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::None, texture);
        const std::array<Vector2, 4> flipped = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::FlipVertically, texture);

        EXPECT_FLOAT_EQ(flipped[0].Y, noFlip[2].Y);
        EXPECT_FLOAT_EQ(flipped[2].Y, noFlip[0].Y);
        EXPECT_FLOAT_EQ(flipped[0].X, noFlip[0].X);
        EXPECT_FLOAT_EQ(flipped[1].Y, noFlip[3].Y);
        EXPECT_FLOAT_EQ(flipped[3].Y, noFlip[1].Y);
    }

    TEST(CalculateTextureCoordinatesTest, FlipDiagonally_TransposesCorners)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 100, 100);
        const Rectangle sourceRect(0, 0, 50, 25);

        const std::array<Vector2, 4> noFlip = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::None, texture);
        const std::array<Vector2, 4> flipped = CalculateTextureCoordinates(sourceRect, TilemapTileFlipFlags::FlipDiagonally, texture);

        // Diagonal flip transposes (x, y): top-right and bottom-left swap places; top-left/bottom-right stay.
        EXPECT_FLOAT_EQ(flipped[0].X, noFlip[0].X);
        EXPECT_FLOAT_EQ(flipped[0].Y, noFlip[0].Y);
        EXPECT_FLOAT_EQ(flipped[1].X, noFlip[2].X);
        EXPECT_FLOAT_EQ(flipped[1].Y, noFlip[2].Y);
        EXPECT_FLOAT_EQ(flipped[2].X, noFlip[1].X);
        EXPECT_FLOAT_EQ(flipped[2].Y, noFlip[1].Y);
        EXPECT_FLOAT_EQ(flipped[3].X, noFlip[3].X);
        EXPECT_FLOAT_EQ(flipped[3].Y, noFlip[3].Y);
    }

    TEST(AddTileQuadTest, AppendsFourVerticesAndSixIndicesWithCcwWinding)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 64, 64);
        std::vector<VertexPositionColorTexture> vertices;
        std::vector<int> indices;

        AddTileQuad(vertices, indices, Vector2(10, 20), 32, 32, Rectangle(0, 0, 32, 32),
                    TilemapTileFlipFlags::None, texture, Color::White);

        ASSERT_EQ(vertices.size(), 4u);
        ASSERT_EQ(indices.size(), 6u);

        EXPECT_FLOAT_EQ(vertices[0].Position.X, 10.0f);
        EXPECT_FLOAT_EQ(vertices[0].Position.Y, 20.0f);
        EXPECT_FLOAT_EQ(vertices[1].Position.X, 42.0f);
        EXPECT_FLOAT_EQ(vertices[1].Position.Y, 20.0f);
        EXPECT_FLOAT_EQ(vertices[2].Position.X, 10.0f);
        EXPECT_FLOAT_EQ(vertices[2].Position.Y, 52.0f);
        EXPECT_FLOAT_EQ(vertices[3].Position.X, 42.0f);
        EXPECT_FLOAT_EQ(vertices[3].Position.Y, 52.0f);

        const std::vector<int> expectedIndices = {0, 1, 2, 1, 3, 2};
        EXPECT_EQ(indices, expectedIndices);
    }

    TEST(AddTileQuadTest, AppendsToExistingVerticesUsingCorrectOffset)
    {
        GraphicsDevice graphicsDevice;
        const Texture2D texture(graphicsDevice, 64, 64);
        std::vector<VertexPositionColorTexture> vertices(4, VertexPositionColorTexture(Vector3::Zero, Color::White, Vector2::Zero));
        std::vector<int> indices;

        AddTileQuad(vertices, indices, Vector2::Zero, 16, 16, Rectangle(0, 0, 16, 16),
                    TilemapTileFlipFlags::None, texture, Color::White);

        ASSERT_EQ(vertices.size(), 8u);
        const std::vector<int> expectedIndices = {4, 5, 6, 5, 7, 6};
        EXPECT_EQ(indices, expectedIndices);
    }

    TEST_F(TilemapRendererSharedTest, CreateLayerModel_SmallIndexCount_UsesSixteenBitIndices)
    {
        Texture2D texture(graphicsDevice, 32, 32);
        std::vector<VertexPositionColorTexture> vertices;
        std::vector<int> indices;
        AddTileQuad(vertices, indices, Vector2::Zero, 16, 16, Rectangle(0, 0, 16, 16),
                    TilemapTileFlipFlags::None, texture, Color::White);

        const LayerModel model = CreateLayerModel(graphicsDevice, vertices, indices, &texture);

        EXPECT_EQ(model.getPrimitiveCountProperty(), 2);
        EXPECT_EQ(model.getIndexBufferProperty().getIndexElementSizeProperty(), Microsoft::Xna::Framework::Graphics::IndexElementSize::SixteenBits);
        EXPECT_EQ(model.getTextureProperty(), &texture);
        EXPECT_EQ(model.getParallaxFactorProperty(), Vector2::One);
    }

    TEST_F(TilemapRendererSharedTest, CreateLayerModel_ParallaxFactorIsSettable)
    {
        Texture2D texture(graphicsDevice, 32, 32);
        std::vector<VertexPositionColorTexture> vertices;
        std::vector<int> indices;
        AddTileQuad(vertices, indices, Vector2::Zero, 16, 16, Rectangle(0, 0, 16, 16),
                    TilemapTileFlipFlags::None, texture, Color::White);

        LayerModel model = CreateLayerModel(graphicsDevice, vertices, indices, &texture);
        model.setParallaxFactorProperty(Vector2(0.5f, 0.25f));

        EXPECT_EQ(model.getParallaxFactorProperty(), Vector2(0.5f, 0.25f));
    }

    TEST(GetWrapSamplerStateTest, PointClamp_MapsToPointWrap)
    {
        EXPECT_EQ(GetWrapSamplerState(&SamplerState::PointClamp), &SamplerState::PointWrap);
    }

    TEST(GetWrapSamplerStateTest, LinearClamp_MapsToLinearWrap)
    {
        EXPECT_EQ(GetWrapSamplerState(&SamplerState::LinearClamp), &SamplerState::LinearWrap);
    }

    TEST(GetWrapSamplerStateTest, AlreadyWrapping_ReturnsUnchanged)
    {
        EXPECT_EQ(GetWrapSamplerState(&SamplerState::PointWrap), &SamplerState::PointWrap);
        EXPECT_EQ(GetWrapSamplerState(&SamplerState::LinearWrap), &SamplerState::LinearWrap);
    }

    TEST(GetWrapSamplerStateTest, OtherPreset_ReturnsUnchanged)
    {
        EXPECT_EQ(GetWrapSamplerState(&SamplerState::AnisotropicClamp), &SamplerState::AnisotropicClamp);
    }
}
