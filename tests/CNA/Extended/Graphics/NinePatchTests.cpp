// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream unit tests exist for NinePatch. Fresh tests below.
#include "CNA/Extended/Graphics/NinePatch.hpp"

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

        Texture2D MakeTestTexture(int width = 30, int height = 30, const std::string& name = "patches")
        {
            Texture2D texture = Texture2D::CreateCpuOnlyForTests(width, height, SurfaceFormat::Color,
                std::vector<Color>(static_cast<std::size_t>(width * height), Color::White));
            texture.setNameProperty(name);
            return texture;
        }

        std::vector<std::shared_ptr<Texture2DRegion>> MakeNinePatches(Texture2D& texture)
        {
            std::vector<std::shared_ptr<Texture2DRegion>> patches(9);
            // TopLeft: 5x5, BottomRight: 7x9 -- distinct sizes so Padding's four values are independently checkable.
            patches[NinePatch::TopLeft] = std::make_shared<Texture2DRegion>(&texture, 0, 0, 5, 5, "tl");
            patches[NinePatch::TopMiddle] = std::make_shared<Texture2DRegion>(&texture, 5, 0, 18, 5, "tm");
            patches[NinePatch::TopRight] = std::make_shared<Texture2DRegion>(&texture, 23, 0, 7, 5, "tr");
            patches[NinePatch::MiddleLeft] = std::make_shared<Texture2DRegion>(&texture, 0, 5, 5, 16, "ml");
            patches[NinePatch::Middle] = std::make_shared<Texture2DRegion>(&texture, 5, 5, 18, 16, "mm");
            patches[NinePatch::MiddleRight] = std::make_shared<Texture2DRegion>(&texture, 23, 5, 7, 16, "mr");
            patches[NinePatch::BottomLeft] = std::make_shared<Texture2DRegion>(&texture, 0, 21, 5, 9, "bl");
            patches[NinePatch::BottomMiddle] = std::make_shared<Texture2DRegion>(&texture, 5, 21, 18, 9, "bm");
            patches[NinePatch::BottomRight] = std::make_shared<Texture2DRegion>(&texture, 23, 21, 7, 9, "br");
            return patches;
        }
    }

    TEST(NinePatchTests, ComputesPaddingFromTopLeftAndBottomRightPatches)
    {
        Texture2D texture = MakeTestTexture();
        const NinePatch ninePatch(MakeNinePatches(texture));

        EXPECT_EQ(ninePatch.getPaddingProperty(), Thickness(5, 5, 7, 9));
    }

    TEST(NinePatchTests, DefaultsNameFromTopLeftPatchTexture)
    {
        Texture2D texture = MakeTestTexture(30, 30, "sheet");
        const NinePatch ninePatch(MakeNinePatches(texture));

        EXPECT_EQ(ninePatch.getNameProperty(), "sheet-nine-patch");
    }

    TEST(NinePatchTests, ExplicitNameIsUsed)
    {
        Texture2D texture = MakeTestTexture();
        const NinePatch ninePatch(MakeNinePatches(texture), "custom");

        EXPECT_EQ(ninePatch.getNameProperty(), "custom");
    }

    TEST(NinePatchTests, PatchesPreserveOrder)
    {
        Texture2D texture = MakeTestTexture();
        const std::vector<std::shared_ptr<Texture2DRegion>> patches = MakeNinePatches(texture);
        const NinePatch ninePatch(patches);

        ASSERT_EQ(ninePatch.getPatchesProperty().size(), 9u);
        EXPECT_EQ(ninePatch.getPatchesProperty()[NinePatch::TopLeft], patches[NinePatch::TopLeft]);
        EXPECT_EQ(ninePatch.getPatchesProperty()[NinePatch::BottomRight], patches[NinePatch::BottomRight]);
    }

    TEST(NinePatchTests, WrongPatchCountThrows)
    {
        Texture2D texture = MakeTestTexture();
        std::vector<std::shared_ptr<Texture2DRegion>> tooFew(3);
        for (std::shared_ptr<Texture2DRegion>& patch : tooFew)
        {
            patch = std::make_shared<Texture2DRegion>(&texture, 0, 0, 1, 1, "p");
        }

        EXPECT_THROW((void)NinePatch(tooFew), std::invalid_argument);
    }
}
