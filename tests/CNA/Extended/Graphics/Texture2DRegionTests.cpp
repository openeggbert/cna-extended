// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for Texture2DRegion itself. Fresh tests below, using
// Texture2D::CreateCpuOnlyForTests (a NOXNA CNA test-only factory building a Texture2D with no
// GraphicsDevice/GPU backend) so these run headlessly.
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"

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

        Texture2D MakeTestTexture(int width = 64, int height = 32, const std::string& name = "test-texture")
        {
            Texture2D texture = Texture2D::CreateCpuOnlyForTests(width, height, SurfaceFormat::Color,
                std::vector<Color>(static_cast<std::size_t>(width * height), Color::White));
            texture.setNameProperty(name);
            return texture;
        }

        // System::Object is abstract (pure-virtual GetTypeName()); a minimal concrete subclass is
        // needed to construct one as a Tag test double.
        struct TestObject final : System::Object
        {
            [[nodiscard]] const std::string& GetTypeName() const override
            {
                static const std::string name = "TestObject";
                return name;
            }
        };
    }

    TEST(Texture2DRegionTests, ConstructFromWholeTextureUsesTextureDimensions)
    {
        Texture2D texture = MakeTestTexture(64, 32);
        const Texture2DRegion region(&texture);

        EXPECT_EQ(region.getXProperty(), 0);
        EXPECT_EQ(region.getYProperty(), 0);
        EXPECT_EQ(region.getWidthProperty(), 64);
        EXPECT_EQ(region.getHeightProperty(), 32);
        EXPECT_EQ(region.getNameProperty(), "test-texture");
        EXPECT_EQ(region.getTextureProperty(), &texture);
    }

    TEST(Texture2DRegionTests, ConstructWithNameParameterIgnoresIt)
    {
        // Upstream bug preserved faithfully: Texture2DRegion(Texture2D, string) never uses its
        // name argument -- see Texture2DRegion.hpp's header comment.
        Texture2D texture = MakeTestTexture(64, 32, "my-texture");
        const Texture2DRegion region(&texture, "explicit-name");

        EXPECT_EQ(region.getNameProperty(), "my-texture");
    }

    TEST(Texture2DRegionTests, ConstructWithRectangleAndName)
    {
        Texture2D texture = MakeTestTexture(64, 32);
        const Texture2DRegion region(&texture, Rectangle(4, 8, 16, 12), "sub");

        EXPECT_EQ(region.getNameProperty(), "sub");
        EXPECT_EQ(region.getXProperty(), 4);
        EXPECT_EQ(region.getYProperty(), 8);
        EXPECT_EQ(region.getWidthProperty(), 16);
        EXPECT_EQ(region.getHeightProperty(), 12);
        EXPECT_EQ(region.getBoundsProperty(), Rectangle(4, 8, 16, 12));
    }

    TEST(Texture2DRegionTests, DefaultsNameFromTextureWhenEmpty)
    {
        Texture2D texture = MakeTestTexture(64, 32, "atlas-texture");
        const Texture2DRegion region(&texture, 0, 0, 16, 16, "");

        EXPECT_EQ(region.getNameProperty(), "atlas-texture");
    }

    TEST(Texture2DRegionTests, ComputesUvCoordinatesFromTextureDimensions)
    {
        Texture2D texture = MakeTestTexture(100, 200);
        const Texture2DRegion region(&texture, 10, 20, 30, 40);

        EXPECT_FLOAT_EQ(region.getLeftUVProperty(), 10.0f / 100.0f);
        EXPECT_FLOAT_EQ(region.getRightUVProperty(), 40.0f / 100.0f);
        EXPECT_FLOAT_EQ(region.getTopUVProperty(), 20.0f / 200.0f);
        EXPECT_FLOAT_EQ(region.getBottomUVProperty(), 60.0f / 200.0f);
    }

    TEST(Texture2DRegionTests, FullConstructorStoresRotationOffsetAndOrigin)
    {
        Texture2D texture = MakeTestTexture(64, 32);
        const Texture2DRegion region(&texture, 0, 0, 16, 16, true, Size(20, 20), Vector2(1.0f, 2.0f), Vector2(0.5f, 0.5f), "rotated");

        EXPECT_TRUE(region.getIsRotatedProperty());
        EXPECT_EQ(region.getOriginalSizeProperty(), Size(20, 20));
        EXPECT_EQ(region.getOffsetProperty(), Vector2(1.0f, 2.0f));
        ASSERT_TRUE(region.getOriginNormalizedProperty().has_value());
        EXPECT_EQ(region.getOriginNormalizedProperty().value(), Vector2(0.5f, 0.5f));
    }

    TEST(Texture2DRegionTests, NullTextureThrows)
    {
        EXPECT_THROW((void)Texture2DRegion(static_cast<Texture2D*>(nullptr)), std::invalid_argument);
    }

    TEST(Texture2DRegionTests, TestOnlyConstructorHasNoTextureAndUnitUv)
    {
        const Texture2DRegion region("test-only", Rectangle(2, 3, 10, 5));

        EXPECT_EQ(region.getTextureProperty(), nullptr);
        EXPECT_EQ(region.getNameProperty(), "test-only");
        EXPECT_EQ(region.getOriginalSizeProperty(), Size(10, 5));
        EXPECT_FLOAT_EQ(region.getLeftUVProperty(), 2.0f);
        EXPECT_FLOAT_EQ(region.getTopUVProperty(), 3.0f);
        EXPECT_FLOAT_EQ(region.getRightUVProperty(), 12.0f);
        EXPECT_FLOAT_EQ(region.getBottomUVProperty(), 8.0f);
        EXPECT_FALSE(region.getOriginNormalizedProperty().has_value());
    }

    TEST(Texture2DRegionTests, TagIsMutable)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DRegion region(&texture);

        EXPECT_EQ(region.getTagProperty(), nullptr);
        TestObject tag;
        region.setTagProperty(&tag);
        EXPECT_EQ(region.getTagProperty(), &tag);
    }

    TEST(Texture2DRegionTests, ToStringIncludesNameAndBounds)
    {
        const Texture2DRegion region("named", Rectangle(1, 2, 3, 4));
        EXPECT_EQ(region.ToString(), "named " + Rectangle(1, 2, 3, 4).ToString());
    }
}
