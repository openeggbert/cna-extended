// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for SpriteSheet, SpriteSheetAnimation, SpriteSheetAnimationFrame,
// or SpriteSheetAnimationBuilder. Fresh tests below; SpriteSheetAnimation/Frame/Builder are
// exercised indirectly through SpriteSheet::DefineAnimation, matching how upstream's own public
// API surfaces them (SpriteSheetAnimationFrame/Builder are `internal` upstream; there is no way
// to construct one except via DefineAnimation's builder callback).
#include "CNA/Extended/Graphics/SpriteSheet.hpp"

#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Graphics/SpriteSheetAnimationBuilder.hpp"
#include "CNA/Extended/Graphics/Texture2DAtlas.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Graphics
{
    namespace
    {
        using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
        using Microsoft::Xna::Framework::Graphics::Texture2D;

        Texture2D MakeTestTexture(int width = 64, int height = 64, const std::string& name = "sheet-texture")
        {
            Texture2D texture = Texture2D::CreateCpuOnlyForTests(width, height, SurfaceFormat::Color,
                std::vector<Color>(static_cast<std::size_t>(width * height), Color::White));
            texture.setNameProperty(name);
            return texture;
        }

        Texture2DAtlas MakeAtlasWithThreeRegions(Texture2D& texture)
        {
            Texture2DAtlas atlas("atlas", &texture);
            atlas.CreateRegion(0, 0, 16, 16, "frame0");
            atlas.CreateRegion(16, 0, 16, 16, "frame1");
            atlas.CreateRegion(32, 0, 16, 16, "frame2");
            return atlas;
        }
    }

    TEST(SpriteSheetTests, ConstructorStoresNameAndAtlas)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeAtlasWithThreeRegions(texture);
        const SpriteSheet spriteSheet("walk", atlas);

        EXPECT_EQ(spriteSheet.getNameProperty(), "walk");
        EXPECT_EQ(&spriteSheet.getTextureAtlasProperty(), &atlas);
        EXPECT_EQ(spriteSheet.getAnimationCountProperty(), 0);
    }

    TEST(SpriteSheetTests, CreateSpriteByIndexAndName)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeAtlasWithThreeRegions(texture);
        const SpriteSheet spriteSheet("sheet", atlas);

        const Sprite byIndex = spriteSheet.CreateSprite(1);
        const Sprite byName = spriteSheet.CreateSprite("frame1");

        EXPECT_EQ(byIndex.getTextureRegionProperty()->getNameProperty(), "frame1");
        EXPECT_EQ(byName.getTextureRegionProperty()->getNameProperty(), "frame1");
    }

    TEST(SpriteSheetTests, DefineAnimationBuildsFramesByIndexAndName)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeAtlasWithThreeRegions(texture);
        SpriteSheet spriteSheet("sheet", atlas);

        spriteSheet.DefineAnimation("walk", [](SpriteSheetAnimationBuilder& builder) {
            builder.AddFrame(0, TimeSpan::FromSeconds(0.1))
                .AddFrame("frame1", TimeSpan::FromSeconds(0.2))
                .IsLooping(true)
                .IsPingPong(true);
        });

        EXPECT_EQ(spriteSheet.getAnimationCountProperty(), 1);

        const std::shared_ptr<SpriteSheetAnimation> animation = spriteSheet.GetAnimation("walk");
        ASSERT_NE(animation, nullptr);
        EXPECT_EQ(animation->getNameProperty(), "walk");
        EXPECT_EQ(animation->getFrameCountProperty(), 2);
        EXPECT_TRUE(animation->getIsLoopingProperty());
        EXPECT_FALSE(animation->getIsReversedProperty());
        EXPECT_TRUE(animation->getIsPingPongProperty());

        ASSERT_EQ(animation->getFramesProperty().size(), 2u);
        EXPECT_EQ(animation->getFramesProperty()[0]->getFrameIndexProperty(), 0);
        EXPECT_EQ(animation->getFramesProperty()[1]->getFrameIndexProperty(), 1);
        EXPECT_EQ(animation->getFramesProperty()[1]->getDurationProperty(), TimeSpan::FromSeconds(0.2));
    }

    TEST(SpriteSheetTests, TryGetAnimationAndRemoveAnimationDefinition)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeAtlasWithThreeRegions(texture);
        SpriteSheet spriteSheet("sheet", atlas);

        spriteSheet.DefineAnimation("idle", [](SpriteSheetAnimationBuilder& builder) { builder.AddFrame(0, TimeSpan::Zero); });

        std::shared_ptr<SpriteSheetAnimation> found;
        EXPECT_TRUE(spriteSheet.TryGetAnimation("idle", found));
        EXPECT_NE(found, nullptr);

        std::shared_ptr<SpriteSheetAnimation> missing;
        EXPECT_FALSE(spriteSheet.TryGetAnimation("missing", missing));
        EXPECT_EQ(missing, nullptr);

        EXPECT_TRUE(spriteSheet.RemoveAnimationDefinition("idle"));
        EXPECT_EQ(spriteSheet.getAnimationCountProperty(), 0);
        EXPECT_FALSE(spriteSheet.RemoveAnimationDefinition("idle"));
    }

    TEST(SpriteSheetTests, GetAnimationMissingNameThrows)
    {
        Texture2D texture = MakeTestTexture();
        Texture2DAtlas atlas = MakeAtlasWithThreeRegions(texture);
        const SpriteSheet spriteSheet("sheet", atlas);

        EXPECT_THROW((void)spriteSheet.GetAnimation("missing"), System::Collections::Generic::KeyNotFoundException);
    }
}
