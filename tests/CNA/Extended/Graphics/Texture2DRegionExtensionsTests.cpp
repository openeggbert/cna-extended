// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for Texture2DRegionExtensions. Fresh tests below.
#include "CNA/Extended/Graphics/Texture2DRegionExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    namespace
    {
        using Microsoft::Xna::Framework::Color;
        using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
        using Microsoft::Xna::Framework::Graphics::Texture2D;

        Texture2D MakeTestTexture(int width = 64, int height = 64, const std::string& name = "atlas-texture")
        {
            Texture2D texture = Texture2D::CreateCpuOnlyForTests(width, height, SurfaceFormat::Color,
                std::vector<Color>(static_cast<std::size_t>(width * height), Color::White));
            texture.setNameProperty(name);
            return texture;
        }
    }

    TEST(Texture2DRegionExtensionsTests, GetSubregionOfUntrimmedRegionReturnsExpectedBounds)
    {
        Texture2D texture = MakeTestTexture(64, 64);
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 32, 32, "region");

        const std::shared_ptr<Texture2DRegion> sub = GetSubregion(region, 4, 4, 8, 8);

        ASSERT_NE(sub, nullptr);
        EXPECT_EQ(sub->getXProperty(), 4);
        EXPECT_EQ(sub->getYProperty(), 4);
        EXPECT_EQ(sub->getWidthProperty(), 8);
        EXPECT_EQ(sub->getHeightProperty(), 8);
        EXPECT_EQ(sub->getTextureProperty(), &texture);
    }

    TEST(Texture2DRegionExtensionsTests, GetSubregionOutsideTrimmedBoundsReturnsNull)
    {
        Texture2D texture = MakeTestTexture(64, 64);
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 8, 8, "region");

        const std::shared_ptr<Texture2DRegion> sub = GetSubregion(region, 100, 100, 8, 8);

        EXPECT_EQ(sub, nullptr);
    }

    TEST(Texture2DRegionExtensionsTests, GetSubregionDefaultsNameFromTextureAndCoordinates)
    {
        Texture2D texture = MakeTestTexture(64, 64, "sheet");
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 32, 32, "region");

        const std::shared_ptr<Texture2DRegion> sub = GetSubregion(region, 1, 2, 3, 4);

        ASSERT_NE(sub, nullptr);
        EXPECT_EQ(sub->getNameProperty(), "sheet(1, 2, 3, 4)");
    }

    TEST(Texture2DRegionExtensionsTests, GetSubregionWithExplicitNameOverload)
    {
        Texture2D texture = MakeTestTexture(64, 64);
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 32, 32, "region");

        const std::shared_ptr<Texture2DRegion> sub = GetSubregion(region, "named-sub", Rectangle(2, 2, 4, 4));

        ASSERT_NE(sub, nullptr);
        EXPECT_EQ(sub->getNameProperty(), "named-sub");
    }

    TEST(Texture2DRegionExtensionsTests, GetSubregionNullRegionThrows)
    {
        EXPECT_THROW((void)GetSubregion(std::shared_ptr<Texture2DRegion>(), Rectangle(0, 0, 1, 1)), std::invalid_argument);
    }

    TEST(Texture2DRegionExtensionsTests, CreateNinePatchUniformPaddingProducesNineSizedPatches)
    {
        Texture2D texture = MakeTestTexture(30, 30);
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 30, 30, "region");

        const std::shared_ptr<NinePatch> ninePatch = CreateNinePatch(region, 5);

        ASSERT_NE(ninePatch, nullptr);
        EXPECT_EQ(ninePatch->getPatchesProperty().size(), 9u);
        EXPECT_EQ(ninePatch->getPatchesProperty()[NinePatch::TopLeft]->getWidthProperty(), 5);
        EXPECT_EQ(ninePatch->getPatchesProperty()[NinePatch::TopLeft]->getHeightProperty(), 5);
        EXPECT_EQ(ninePatch->getPatchesProperty()[NinePatch::Middle]->getWidthProperty(), 20);
        EXPECT_EQ(ninePatch->getPatchesProperty()[NinePatch::Middle]->getHeightProperty(), 20);
    }

    TEST(Texture2DRegionExtensionsTests, CreateNinePatchWithThicknessMatchesFourArgOverload)
    {
        Texture2D texture = MakeTestTexture(40, 40);
        const auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 40, 40, "region");

        const std::shared_ptr<NinePatch> viaThickness = CreateNinePatch(region, Thickness(1, 2, 3, 4));
        const std::shared_ptr<NinePatch> viaFourArgs = CreateNinePatch(region, 1, 2, 3, 4);

        ASSERT_NE(viaThickness, nullptr);
        ASSERT_NE(viaFourArgs, nullptr);
        EXPECT_EQ(viaThickness->getPaddingProperty(), viaFourArgs->getPaddingProperty());
    }

    TEST(Texture2DRegionExtensionsTests, CreateNinePatchNullRegionThrows)
    {
        EXPECT_THROW((void)CreateNinePatch(std::shared_ptr<Texture2DRegion>(), 1), std::invalid_argument);
    }
}
