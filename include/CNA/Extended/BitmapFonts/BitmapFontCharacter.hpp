// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BitmapFonts/BitmapFontCharacter.cs. `TextureRegion` is
// `std::shared_ptr<Texture2DRegion>`, matching this project's established Texture2DRegion
// ownership convention (see Graphics/Texture2DRegion.hpp's header comment). `Kernings` (a
// `Dictionary<int,int>` in C#) reuses `System::Collections::Generic::Dictionary`, matching this
// project's established Dictionary-reuse convention (see Graphics/Texture2DAtlas.hpp).
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "System/Collections/Generic/Dictionary.hpp"

#include <memory>

namespace CNA::Extended::BitmapFonts
{
    using CNA::Extended::Graphics::Texture2DRegion;

    /** @brief Represents a character in a bitmap font. */
    class BitmapFontCharacter
    {
    public:
        /**
         * @brief Initializes a new BitmapFontCharacter.
         * @param character The character code.
         * @param textureRegion The texture region that contains the character's image.
         * @param xOffset The horizontal offset for rendering the character.
         * @param yOffset The vertical offset for rendering the character.
         * @param xAdvance The horizontal advance value for rendering the next character.
         */
        BitmapFontCharacter(int character, std::shared_ptr<Texture2DRegion> textureRegion, int xOffset, int yOffset, int xAdvance)
            : character_(character), textureRegion_(std::move(textureRegion)), xOffset_(xOffset), yOffset_(yOffset), xAdvance_(xAdvance)
        {
        }

        /** @return The character code. */
        [[nodiscard]] int getCharacterProperty() const { return character_; }

        /** @return The texture region that contains the character's image. */
        [[nodiscard]] const std::shared_ptr<Texture2DRegion>& getTextureRegionProperty() const { return textureRegion_; }

        /** @return The horizontal offset for rendering the character. */
        [[nodiscard]] int getXOffsetProperty() const { return xOffset_; }

        /** @return The vertical offset for rendering the character. */
        [[nodiscard]] int getYOffsetProperty() const { return yOffset_; }

        /** @return The horizontal advance value for rendering the next character. */
        [[nodiscard]] int getXAdvanceProperty() const { return xAdvance_; }

        /** @return The dictionary of kerning values for pairs of characters, keyed by the following character's code. */
        [[nodiscard]] System::Collections::Generic::Dictionary<int, int>& getKerningsProperty() { return kernings_; }
        [[nodiscard]] const System::Collections::Generic::Dictionary<int, int>& getKerningsProperty() const { return kernings_; }

    private:
        int character_;
        std::shared_ptr<Texture2DRegion> textureRegion_;
        int xOffset_;
        int yOffset_;
        int xAdvance_;
        System::Collections::Generic::Dictionary<int, int> kernings_;
    };
}
