// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for Sprite. Fresh tests below.
#include "CNA/Extended/Graphics/Sprite.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    namespace
    {
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

    TEST(SpriteTests, ConstructFromTextureDefaultsMatchUpstream)
    {
        Texture2D texture = MakeTestTexture();
        const Sprite sprite(&texture);

        EXPECT_TRUE(sprite.getIsVisibleProperty());
        EXPECT_EQ(sprite.getColorProperty(), Color::White);
        EXPECT_FLOAT_EQ(sprite.getAlphaProperty(), 1.0f);
        EXPECT_FLOAT_EQ(sprite.getDepthProperty(), 0.0f);
        EXPECT_EQ(sprite.getEffectProperty(), SpriteEffects::None);
        EXPECT_EQ(sprite.getTagProperty(), nullptr);
        EXPECT_EQ(sprite.getSizeProperty(), Point(64, 32));
    }

    TEST(SpriteTests, ConstructFromRegionSharesTheRegion)
    {
        Texture2D texture = MakeTestTexture();
        auto region = std::make_shared<Texture2DRegion>(&texture, 0, 0, 16, 16, "region");
        const Sprite sprite(region);

        EXPECT_EQ(sprite.getTextureRegionProperty(), region);
    }

    TEST(SpriteTests, NullTextureRegionThrows)
    {
        EXPECT_THROW((void)Sprite(std::shared_ptr<Texture2DRegion>()), std::invalid_argument);
    }

    TEST(SpriteTests, OriginNormalizedRoundTrips)
    {
        Texture2D texture = MakeTestTexture(100, 50);
        Sprite sprite(&texture);

        sprite.setOriginNormalizedProperty(Vector2(0.5f, 0.25f));

        EXPECT_FLOAT_EQ(sprite.getOriginProperty().X, 50.0f);
        EXPECT_FLOAT_EQ(sprite.getOriginProperty().Y, 12.5f);

        const Vector2 normalized = sprite.getOriginNormalizedProperty();
        EXPECT_FLOAT_EQ(normalized.X, 0.5f);
        EXPECT_FLOAT_EQ(normalized.Y, 0.25f);
    }

    TEST(SpriteTests, SetTextureRegionAppliesRegionsNormalizedOrigin)
    {
        Texture2D texture = MakeTestTexture(100, 50);
        Sprite sprite(&texture);

        auto region = std::make_shared<Texture2DRegion>(
            &texture, 0, 0, 100, 50, false, Size(100, 50), Vector2::Zero, Vector2(0.5f, 0.5f), "centered");
        sprite.setTextureRegionProperty(region);

        EXPECT_EQ(sprite.getTextureRegionProperty(), region);
        EXPECT_FLOAT_EQ(sprite.getOriginProperty().X, 50.0f);
        EXPECT_FLOAT_EQ(sprite.getOriginProperty().Y, 25.0f);
    }

    TEST(SpriteTests, SetTextureRegionNullThrows)
    {
        Texture2D texture = MakeTestTexture();
        Sprite sprite(&texture);
        EXPECT_THROW(sprite.setTextureRegionProperty(std::shared_ptr<Texture2DRegion>()), std::invalid_argument);
    }

    TEST(SpriteTests, GetCornersWithNoRotationOrScale)
    {
        Texture2D texture = MakeTestTexture(10, 10);
        const Sprite sprite(&texture);

        const std::array<Vector2, 4> corners = sprite.GetCorners(Vector2::Zero, 0.0f, Vector2::One);

        EXPECT_EQ(corners[0], Vector2(0.0f, 0.0f));
        EXPECT_EQ(corners[1], Vector2(10.0f, 0.0f));
        EXPECT_EQ(corners[2], Vector2(10.0f, 10.0f));
        EXPECT_EQ(corners[3], Vector2(0.0f, 10.0f));
    }

    TEST(SpriteTests, GetBoundingRectangleMatchesTextureSizeAtOrigin)
    {
        Texture2D texture = MakeTestTexture(10, 20);
        const Sprite sprite(&texture);

        const RectangleF bounds = sprite.GetBoundingRectangle(Vector2(5.0f, 5.0f), 0.0f, Vector2::One);

        EXPECT_FLOAT_EQ(bounds.X, 5.0f);
        EXPECT_FLOAT_EQ(bounds.Y, 5.0f);
        EXPECT_FLOAT_EQ(bounds.Width, 10.0f);
        EXPECT_FLOAT_EQ(bounds.Height, 20.0f);
    }

    TEST(SpriteTests, CloneSharesTextureRegionAndCopiesScalarState)
    {
        Texture2D texture = MakeTestTexture();
        Sprite sprite(&texture);
        sprite.setAlphaProperty(0.5f);
        sprite.setDepthProperty(0.75f);
        TestObject tag;
        sprite.setTagProperty(&tag);

        const Sprite clone = sprite.Clone();

        EXPECT_EQ(clone.getTextureRegionProperty(), sprite.getTextureRegionProperty());
        EXPECT_FLOAT_EQ(clone.getAlphaProperty(), 0.5f);
        EXPECT_FLOAT_EQ(clone.getDepthProperty(), 0.75f);
        EXPECT_EQ(clone.getTagProperty(), &tag);
    }
}
