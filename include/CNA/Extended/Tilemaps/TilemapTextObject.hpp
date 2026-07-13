// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Tilemaps/TilemapObjects/TilemapTextObject.cs. Text objects
// store text display information only; rendering is the caller's responsibility, matching
// upstream's own remark.
#pragma once

#include "CNA/Extended/Tilemaps/TilemapObject.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"

#include <string>

namespace CNA::Extended::Tilemaps
{
    using Microsoft::Xna::Framework::Color;

    /** @brief Horizontal text alignment for a TilemapTextObject. */
    enum class TilemapTextObjectHorizontalAlignment
    {
        Left,
        Center,
        Right,
        Justify
    };

    /** @brief Vertical text alignment for a TilemapTextObject. */
    enum class TilemapTextObjectVerticalAlignment
    {
        Top,
        Center,
        Bottom
    };

    /** @brief A text object in a tilemap. Stores text display information only; rendering is the caller's responsibility. */
    class TilemapTextObject : public TilemapObject
    {
    public:
        /** @brief Creates a text object with @p id, @p position, bounding @p size, and @p text content. */
        TilemapTextObject(int id, const Vector2& position, const Vector2& size, std::string text);

        [[nodiscard]] const Vector2& getSizeProperty() const { return size_; }
        void setSizeProperty(const Vector2& value) { size_ = value; }

        [[nodiscard]] const std::string& getTextProperty() const { return text_; }
        void setTextProperty(std::string value) { text_ = std::move(value); }

        [[nodiscard]] const std::string& getFontFamilyProperty() const { return fontFamily_; }
        void setFontFamilyProperty(std::string value) { fontFamily_ = std::move(value); }

        [[nodiscard]] int getPixelSizeProperty() const { return pixelSize_; }
        void setPixelSizeProperty(int value) { pixelSize_ = value; }

        [[nodiscard]] bool getBoldProperty() const { return bold_; }
        void setBoldProperty(bool value) { bold_ = value; }

        [[nodiscard]] bool getItalicProperty() const { return italic_; }
        void setItalicProperty(bool value) { italic_ = value; }

        [[nodiscard]] bool getUnderlineProperty() const { return underline_; }
        void setUnderlineProperty(bool value) { underline_ = value; }

        [[nodiscard]] bool getStrikethroughProperty() const { return strikethrough_; }
        void setStrikethroughProperty(bool value) { strikethrough_ = value; }

        [[nodiscard]] const Color& getColorProperty() const { return color_; }
        void setColorProperty(const Color& value) { color_ = value; }

        [[nodiscard]] bool getWordWrapProperty() const { return wordWrap_; }
        void setWordWrapProperty(bool value) { wordWrap_ = value; }

        [[nodiscard]] TilemapTextObjectHorizontalAlignment getHorizontalAlignProperty() const { return horizontalAlign_; }
        void setHorizontalAlignProperty(TilemapTextObjectHorizontalAlignment value) { horizontalAlign_ = value; }

        [[nodiscard]] TilemapTextObjectVerticalAlignment getVerticalAlignProperty() const { return verticalAlign_; }
        void setVerticalAlignProperty(TilemapTextObjectVerticalAlignment value) { verticalAlign_ = value; }

        [[nodiscard]] BoundingBox2D getBoundsProperty() const override
        {
            return BoundingBox2D::CreateFromPositionAndSize(getPositionProperty(), size_);
        }

    private:
        Vector2 size_;
        std::string text_;
        std::string fontFamily_ = "sans-serif";
        int pixelSize_ = 16;
        bool bold_ = false;
        bool italic_ = false;
        bool underline_ = false;
        bool strikethrough_ = false;
        Color color_ = Color::Black;
        bool wordWrap_ = false;
        TilemapTextObjectHorizontalAlignment horizontalAlign_ = TilemapTextObjectHorizontalAlignment::Left;
        TilemapTextObjectVerticalAlignment verticalAlign_ = TilemapTextObjectVerticalAlignment::Top;
    };
}
