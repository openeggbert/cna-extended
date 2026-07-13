// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for AnimatedSprite. Fresh tests below.
#include "CNA/Extended/Graphics/AnimatedSprite.hpp"

#include "CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    namespace
    {
        using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
        using Microsoft::Xna::Framework::Graphics::Texture2D;

        Texture2D MakeTestTexture(int width = 48, int height = 16, const std::string& name = "anim-texture")
        {
            Texture2D texture = Texture2D::CreateCpuOnlyForTests(width, height, SurfaceFormat::Color,
                std::vector<Color>(static_cast<std::size_t>(width * height), Color::White));
            texture.setNameProperty(name);
            return texture;
        }

        Texture2DAtlas MakeThreeFrameAtlas(Texture2D& texture)
        {
            Texture2DAtlas atlas("atlas", &texture);
            atlas.CreateRegion(0, 0, 16, 16, "frame0");
            atlas.CreateRegion(16, 0, 16, 16, "frame1");
            atlas.CreateRegion(32, 0, 16, 16, "frame2");
            return atlas;
        }

        SpriteSheet MakeWalkSpriteSheet(Texture2DAtlas& atlas)
        {
            SpriteSheet spriteSheet("sheet", atlas);
            spriteSheet.DefineAnimation("walk", [](SpriteSheetAnimationBuilder& builder) {
                builder.AddFrame(0, TimeSpan::FromSeconds(0.1))
                    .AddFrame(1, TimeSpan::FromSeconds(0.1))
                    .AddFrame(2, TimeSpan::FromSeconds(0.1))
                    .IsLooping(true);
            });
            return spriteSheet;
        }
    }

    TEST(AnimatedSpriteTests, ConstructedFromSpriteSheetShowsFirstRegion)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        const AnimatedSprite sprite(spriteSheet);

        EXPECT_EQ(sprite.getTextureRegionProperty()->getNameProperty(), "frame0");
    }

    TEST(AnimatedSpriteTests, AccessingControllerBeforeSetAnimationThrows)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        const AnimatedSprite sprite(spriteSheet);

        EXPECT_THROW((void)sprite.getControllerProperty(), std::logic_error);
        EXPECT_THROW((void)sprite.getCurrentAnimationProperty(), std::logic_error);
    }

    TEST(AnimatedSpriteTests, ConstructorWithInitialAnimationStartsPlaying)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        AnimatedSprite sprite(spriteSheet, "walk");

        EXPECT_EQ(sprite.getCurrentAnimationProperty(), "walk");
        EXPECT_TRUE(sprite.getControllerProperty().getIsAnimatingProperty());
        EXPECT_EQ(sprite.getTextureRegionProperty()->getNameProperty(), "frame0");
    }

    TEST(AnimatedSpriteTests, SetAnimationUpdatesTextureRegionAndController)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        AnimatedSprite sprite(spriteSheet);
        Animations::IAnimationController& controller = sprite.SetAnimation("walk");

        EXPECT_EQ(&controller, &sprite.getControllerProperty());
        EXPECT_EQ(sprite.getCurrentAnimationProperty(), "walk");
        EXPECT_EQ(sprite.getTextureRegionProperty()->getNameProperty(), "frame0");
    }

    TEST(AnimatedSpriteTests, UpdateAdvancesTextureRegionWhenFrameChanges)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        AnimatedSprite sprite(spriteSheet, "walk");
        ASSERT_EQ(sprite.getTextureRegionProperty()->getNameProperty(), "frame0");

        sprite.Update(TimeSpan::FromSeconds(0.15));

        EXPECT_EQ(sprite.getTextureRegionProperty()->getNameProperty(), "frame1");
    }

    TEST(AnimatedSpriteTests, UpdateBeforeSetAnimationThrows)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeThreeFrameAtlas(texture);
        SpriteSheet spriteSheet = MakeWalkSpriteSheet(atlas);

        AnimatedSprite sprite(spriteSheet);

        EXPECT_THROW(sprite.Update(TimeSpan::FromSeconds(0.1)), std::logic_error);
    }
}
