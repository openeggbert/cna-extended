// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for Content/TexturePacker/*. Fresh tests below.
#include "CNA/Extended/Content/TexturePacker/TexturePackerFileContent.hpp"

#include "System/Text/Json/JsonSerializer.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Content::TexturePacker
{
    namespace
    {
        const char* const kFullDocument = R"({
            "frames": [
                {
                    "filename": "player_idle_0.png",
                    "frame": {"x": 0, "y": 0, "w": 32, "h": 32},
                    "rotated": false,
                    "trimmed": true,
                    "spriteSourceSize": {"x": 2, "y": 3, "w": 32, "h": 32},
                    "sourceSize": {"w": 36, "h": 38},
                    "pivot": {"x": 0.5, "y": 0.5}
                }
            ],
            "textures": [
                {
                    "filename": "atlas.png",
                    "format": "RGBA8888",
                    "size": {"w": 128, "h": 128},
                    "scale": "1",
                    "premultiplied": true,
                    "frames": {
                        "enemy_0.png": {
                            "frame": {"x": 32, "y": 0, "w": 16, "h": 16},
                            "rotated": 0,
                            "size": {"w": 16, "h": 16},
                            "offset": {"x": 0, "y": 0},
                            "pivot": {"x": 0.5, "y": 0.5},
                            "scale9": {"x": 1, "y": 1, "w": 14, "h": 14}
                        }
                    }
                }
            ],
            "meta": {
                "app": "http://www.codeandweb.com/texturepacker",
                "version": "1.0",
                "image": "atlas.png",
                "dataformat": "json",
                "smartupdate": "abc123"
            }
        })";

        const char* const kMinimalTextureDocument = R"({
            "frames": [],
            "textures": [
                {
                    "filename": "atlas2.png"
                }
            ],
            "meta": {
                "app": "http://www.codeandweb.com/texturepacker",
                "version": "1.0",
                "image": "atlas2.png",
                "dataformat": "json",
                "smartupdate": ""
            }
        })";
    }

    TEST(TexturePackerFileContentTests, DeserializesTopLevelFramesArray)
    {
        const TexturePackerFileContent content = System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(kFullDocument);

        ASSERT_EQ(content.getRegionsProperty().size(), 1u);
        const TexturePackerFrame& frame = content.getRegionsProperty()[0];
        EXPECT_EQ(frame.getFileNameProperty(), "player_idle_0.png");
        EXPECT_EQ(frame.getFrameProperty().getXProperty(), 0);
        EXPECT_EQ(frame.getFrameProperty().getWidthProperty(), 32);
        EXPECT_FALSE(frame.getRotatedProperty());
        EXPECT_TRUE(frame.getTrimmedProperty());
        EXPECT_EQ(frame.getSpriteSourceSizeProperty().getYProperty(), 3);
        EXPECT_EQ(frame.getSourceSizeProperty().getWidthProperty(), 36);
        EXPECT_EQ(frame.getSourceSizeProperty().getHeightProperty(), 38);
        EXPECT_DOUBLE_EQ(frame.getPivotPointProperty().getXProperty(), 0.5);
        EXPECT_DOUBLE_EQ(frame.getPivotPointProperty().getYProperty(), 0.5);
    }

    TEST(TexturePackerFileContentTests, DeserializesTexturesArrayWithNestedFramesDictionary)
    {
        const TexturePackerFileContent content = System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(kFullDocument);

        ASSERT_EQ(content.getTexturesProperty().size(), 1u);
        const TexturePackerTexture& texture = content.getTexturesProperty()[0];
        EXPECT_EQ(texture.getFileNameProperty(), "atlas.png");
        ASSERT_TRUE(texture.getFormatProperty().has_value());
        EXPECT_EQ(*texture.getFormatProperty(), "RGBA8888");
        ASSERT_TRUE(texture.getSizeProperty().has_value());
        EXPECT_EQ(texture.getSizeProperty()->getWidthProperty(), 128);
        ASSERT_TRUE(texture.getScaleProperty().has_value());
        EXPECT_EQ(*texture.getScaleProperty(), "1");
        EXPECT_TRUE(texture.getPremultipliedProperty());

        ASSERT_TRUE(texture.getFramesProperty().has_value());
        ASSERT_TRUE(texture.getFramesProperty()->ContainsKey("enemy_0.png"));
        const TexturePackerTextureFrame& nested = texture.getFramesProperty()->operator[]("enemy_0.png");
        EXPECT_EQ(nested.getFrameProperty().getXProperty(), 32);
        EXPECT_EQ(nested.getRotatedProperty(), 0);
        ASSERT_TRUE(nested.getSizeProperty().has_value());
        EXPECT_EQ(nested.getSizeProperty()->getWidthProperty(), 16);
        ASSERT_TRUE(nested.getOffsetProperty().has_value());
        EXPECT_EQ(nested.getOffsetProperty()->getXProperty(), 0);
        ASSERT_TRUE(nested.getPivotProperty().has_value());
        EXPECT_DOUBLE_EQ(nested.getPivotProperty()->getYProperty(), 0.5);
        ASSERT_TRUE(nested.getScale9Property().has_value());
        EXPECT_EQ(nested.getScale9Property()->getWidthProperty(), 14);
    }

    TEST(TexturePackerFileContentTests, DeserializesMetaBlock)
    {
        const TexturePackerFileContent content = System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(kFullDocument);

        EXPECT_EQ(content.getMetaProperty().getAppProperty(), "http://www.codeandweb.com/texturepacker");
        EXPECT_EQ(content.getMetaProperty().getVersionProperty(), "1.0");
        EXPECT_EQ(content.getMetaProperty().getImageProperty(), "atlas.png");
        EXPECT_EQ(content.getMetaProperty().getDataFormatProperty(), "json");
        EXPECT_EQ(content.getMetaProperty().getSmartUpdateProperty(), "abc123");
    }

    TEST(TexturePackerFileContentTests, OptionalTextureFieldsAreNulloptWhenAbsent)
    {
        const TexturePackerFileContent content =
            System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(kMinimalTextureDocument);

        ASSERT_EQ(content.getTexturesProperty().size(), 1u);
        const TexturePackerTexture& texture = content.getTexturesProperty()[0];
        EXPECT_EQ(texture.getFileNameProperty(), "atlas2.png");
        EXPECT_FALSE(texture.getFormatProperty().has_value());
        EXPECT_FALSE(texture.getSizeProperty().has_value());
        EXPECT_FALSE(texture.getScaleProperty().has_value());
        EXPECT_FALSE(texture.getPremultipliedProperty());
        EXPECT_FALSE(texture.getFramesProperty().has_value());
    }

    TEST(TexturePackerFileContentTests, ThrowsOnMissingRequiredFrameField)
    {
        const char* const missingFileName = R"({
            "frames": [
                {
                    "frame": {"x": 0, "y": 0, "w": 1, "h": 1},
                    "rotated": false,
                    "trimmed": false,
                    "spriteSourceSize": {"x": 0, "y": 0, "w": 1, "h": 1},
                    "sourceSize": {"w": 1, "h": 1},
                    "pivot": {"x": 0, "y": 0}
                }
            ],
            "textures": [],
            "meta": {"app": "a", "version": "1", "image": "i", "dataformat": "json", "smartupdate": ""}
        })";

        EXPECT_THROW((void)System::Text::Json::JsonSerializer::Deserialize<TexturePackerFileContent>(missingFileName),
                     System::Text::Json::JsonException);
    }
}
