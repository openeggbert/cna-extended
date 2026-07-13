// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BitmapFonts/BitmapFont.hpp"

#include "CNA/Extended/Content/BitmapFonts/BitmapFontFileReader.hpp"
#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/TitleContainer.hpp"
#include "System/Collections/Generic/Dictionary.hpp"
#include "System/IO/Path.hpp"

#include <utility>

namespace CNA::Extended::BitmapFonts
{
    namespace
    {
        // Decodes one Unicode code point from UTF-8-encoded `text` starting at `index`, advancing
        // `index` to the LAST byte consumed (not one past it) -- matching upstream's
        // GetUnicodeCodePoint(string, ref int index) contract, where `index` ends up pointing at
        // the last UTF-16 unit consumed so the caller's own `++index` advances correctly to the
        // next code point. Translates "surrogate pair decoding" (a UTF-16 concept) into "multi-byte
        // UTF-8 decoding": both exist to reconstruct one full code point from more than one storage
        // unit, but the mechanisms differ because this project's strings are UTF-8, not UTF-16 --
        // see BitmapFont.hpp's header comment. Unlike SpriteFont's BMP-only
        // CNA::Internal::DecodeUtf8CodePoint (which this intentionally does not reuse -- it's a
        // private header of the sibling `cna` repo, and is scoped to a char16_t-keyed glyph map),
        // this returns the full code point since BitmapFont's character dictionary is int-keyed and
        // can represent code points beyond the Basic Multilingual Plane. Invalid/truncated sequences
        // decode to '?' and always leave `index` such that the caller's `++index` advances by at
        // least one byte, so malformed input can't spin forever.
        int DecodeUtf8CodePointAt(const std::string& text, std::size_t& index)
        {
            const auto lead = static_cast<unsigned char>(text[index]);
            if (lead < 0x80)
            {
                return lead;
            }

            int extraBytes;
            char32_t codepoint;
            if ((lead & 0xE0) == 0xC0)
            {
                extraBytes = 1;
                codepoint = lead & 0x1Fu;
            }
            else if ((lead & 0xF0) == 0xE0)
            {
                extraBytes = 2;
                codepoint = lead & 0x0Fu;
            }
            else if ((lead & 0xF8) == 0xF0)
            {
                extraBytes = 3;
                codepoint = lead & 0x07u;
            }
            else
            {
                return '?';
            }

            if (index + static_cast<std::size_t>(extraBytes) >= text.size())
            {
                return '?';
            }

            char32_t decoded = codepoint;
            for (int k = 1; k <= extraBytes; ++k)
            {
                const auto cont = static_cast<unsigned char>(text[index + static_cast<std::size_t>(k)]);
                if ((cont & 0xC0) != 0x80)
                {
                    return '?';
                }
                decoded = (decoded << 6) | (cont & 0x3Fu);
            }

            index += static_cast<std::size_t>(extraBytes);
            return static_cast<int>(decoded);
        }

        int DecodeUtf8CodePointAt(const System::Text::StringBuilder& text, std::size_t& index)
        {
            const auto lead = static_cast<unsigned char>(text[static_cast<SharpRuntime::intcs>(index)]);
            if (lead < 0x80)
            {
                return lead;
            }

            int extraBytes;
            char32_t codepoint;
            if ((lead & 0xE0) == 0xC0)
            {
                extraBytes = 1;
                codepoint = lead & 0x1Fu;
            }
            else if ((lead & 0xF0) == 0xE0)
            {
                extraBytes = 2;
                codepoint = lead & 0x0Fu;
            }
            else if ((lead & 0xF8) == 0xF0)
            {
                extraBytes = 3;
                codepoint = lead & 0x07u;
            }
            else
            {
                return '?';
            }

            if (index + static_cast<std::size_t>(extraBytes) >= static_cast<std::size_t>(text.getLengthProperty()))
            {
                return '?';
            }

            char32_t decoded = codepoint;
            for (int k = 1; k <= extraBytes; ++k)
            {
                const auto cont = static_cast<unsigned char>(text[static_cast<SharpRuntime::intcs>(index + static_cast<std::size_t>(k))]);
                if ((cont & 0xC0) != 0x80)
                {
                    return '?';
                }
                decoded = (decoded << 6) | (cont & 0x3Fu);
            }

            index += static_cast<std::size_t>(extraBytes);
            return static_cast<int>(decoded);
        }
    }

    BitmapFont::BitmapFont(std::string face, int size, int lineHeight, const std::vector<std::shared_ptr<BitmapFontCharacter>>& characters)
        : BitmapFont(std::move(face), size, lineHeight, 0, 0, characters)
    {
    }

    BitmapFont::BitmapFont(std::string face, int size, int lineHeight, int letterSpacing, int lineSpacing,
        const std::vector<std::shared_ptr<BitmapFontCharacter>>& characters)
        : face_(std::move(face)), size_(size), lineHeight_(lineHeight), letterSpacing_(letterSpacing), lineSpacing_(lineSpacing)
    {
        for (const std::shared_ptr<BitmapFontCharacter>& character : characters)
        {
            characters_.Add(character->getCharacterProperty(), character);
        }
    }

    std::shared_ptr<BitmapFontCharacter> BitmapFont::GetCharacter(int character) const
    {
        std::shared_ptr<BitmapFontCharacter> value;
        characters_.TryGetValue(character, value);
        return value;
    }

    bool BitmapFont::TryGetCharacter(int character, std::shared_ptr<BitmapFontCharacter>& value) const
    {
        return characters_.TryGetValue(character, value);
    }

    SizeF BitmapFont::MeasureString(const std::string& text) const
    {
        if (text.empty())
        {
            return SizeF::Empty;
        }

        const RectangleF stringRectangle = GetStringRectangle(text);
        return SizeF(stringRectangle.Width, stringRectangle.Height);
    }

    SizeF BitmapFont::MeasureString(const System::Text::StringBuilder* text) const
    {
        if (text == nullptr || text->getLengthProperty() == 0)
        {
            return SizeF::Empty;
        }

        const RectangleF stringRectangle = GetStringRectangle(text);
        return SizeF(stringRectangle.Width, stringRectangle.Height);
    }

    RectangleF BitmapFont::GetStringRectangle(const std::string& text) const
    {
        return GetStringRectangle(text, Vector2());
    }

    RectangleF BitmapFont::GetStringRectangle(const std::string& text, const Vector2& position) const
    {
        RectangleF rectangle(position.X, position.Y, 0, static_cast<float>(lineHeight_));

        for (const BitmapFontGlyph& glyph : GetGlyphs(text, position))
        {
            if (glyph.Character)
            {
                const float right = glyph.Position.X + static_cast<float>(glyph.Character->getTextureRegionProperty()->getWidthProperty());
                if (right > rectangle.getRightProperty())
                {
                    rectangle.Width = right - rectangle.getLeftProperty();
                }
            }

            if (glyph.CharacterID == '\n')
            {
                rectangle.Height += static_cast<float>(lineHeight_);
            }
        }

        return rectangle;
    }

    RectangleF BitmapFont::GetStringRectangle(const System::Text::StringBuilder* text, const std::optional<Vector2>& position) const
    {
        const Vector2 position1 = position.value_or(Vector2());
        RectangleF rectangle(position1.X, position1.Y, 0, static_cast<float>(lineHeight_));

        for (const BitmapFontGlyph& glyph : GetGlyphs(text, position1))
        {
            if (glyph.Character)
            {
                const float right = glyph.Position.X + static_cast<float>(glyph.Character->getTextureRegionProperty()->getWidthProperty());
                if (right > rectangle.getRightProperty())
                {
                    rectangle.Width = right - rectangle.getLeftProperty();
                }
            }

            if (glyph.CharacterID == '\n')
            {
                rectangle.Height += static_cast<float>(lineHeight_ + lineSpacing_);
            }
        }

        return rectangle;
    }

    BitmapFont::StringGlyphEnumerable BitmapFont::GetGlyphs(const std::string& text, const std::optional<Vector2>& position) const
    {
        return StringGlyphEnumerable(this, &text, position.value_or(Vector2()));
    }

    BitmapFont::StringBuilderGlyphEnumerable BitmapFont::GetGlyphs(const System::Text::StringBuilder* text, const std::optional<Vector2>& position) const
    {
        return StringBuilderGlyphEnumerable(this, text, position.value_or(Vector2()));
    }

    // --- StringGlyphIterator -----------------------------------------------------------------------

    BitmapFont::StringGlyphIterator::StringGlyphIterator(const BitmapFont* font, const std::string* text, const Vector2& position)
        : font_(font), text_(text), index_(static_cast<std::size_t>(-1)), position_(position), done_(false)
    {
    }

    bool BitmapFont::StringGlyphIterator::MoveNext()
    {
        ++index_; // wraps SIZE_MAX -> 0 on the first call, matching upstream's `_index = -1` then `++_index`
        if (index_ >= text_->size())
        {
            return false;
        }

        const int character = DecodeUtf8CodePointAt(*text_, index_);
        currentGlyph_.CharacterID = character;
        currentGlyph_.Character = font_->GetCharacter(character);
        currentGlyph_.Position = position_ + positionDelta_;

        if (currentGlyph_.Character)
        {
            currentGlyph_.Position.X += static_cast<float>(currentGlyph_.Character->getXOffsetProperty());
            currentGlyph_.Position.Y += static_cast<float>(currentGlyph_.Character->getYOffsetProperty());
            positionDelta_.X += static_cast<float>(currentGlyph_.Character->getXAdvanceProperty() + font_->getLetterSpacingProperty());
        }

        if (font_->getUseKerningsProperty() && previousGlyph_.has_value() && previousGlyph_->Character)
        {
            int amount = 0;
            if (previousGlyph_->Character->getKerningsProperty().TryGetValue(character, amount))
            {
                positionDelta_.X += static_cast<float>(amount);
                currentGlyph_.Position.X += static_cast<float>(amount);
            }
        }

        previousGlyph_ = currentGlyph_;

        if (character != '\n')
        {
            return true;
        }

        positionDelta_.Y += static_cast<float>(font_->getLineHeightProperty() + font_->getLineSpacingProperty());
        positionDelta_.X = 0.0f;
        previousGlyph_.reset();

        return true;
    }

    BitmapFont::StringGlyphIterator& BitmapFont::StringGlyphIterator::operator++()
    {
        done_ = !MoveNext();
        return *this;
    }

    BitmapFont::StringGlyphIterator BitmapFont::StringGlyphEnumerable::begin() const
    {
        StringGlyphIterator it(font_, text_, position_);
        ++it;
        return it;
    }

    // --- StringBuilderGlyphIterator ----------------------------------------------------------------

    BitmapFont::StringBuilderGlyphIterator::StringBuilderGlyphIterator(
        const BitmapFont* font, const System::Text::StringBuilder* text, const Vector2& position)
        : font_(font), text_(text), index_(static_cast<std::size_t>(-1)), position_(position), done_(false)
    {
    }

    bool BitmapFont::StringBuilderGlyphIterator::MoveNext()
    {
        ++index_;
        if (index_ >= static_cast<std::size_t>(text_->getLengthProperty()))
        {
            return false;
        }

        const int character = DecodeUtf8CodePointAt(*text_, index_);
        currentGlyph_.CharacterID = character;
        currentGlyph_.Character = font_->GetCharacter(character);
        currentGlyph_.Position = position_ + positionDelta_;

        if (currentGlyph_.Character)
        {
            currentGlyph_.Position.X += static_cast<float>(currentGlyph_.Character->getXOffsetProperty());
            currentGlyph_.Position.Y += static_cast<float>(currentGlyph_.Character->getYOffsetProperty());
            positionDelta_.X += static_cast<float>(currentGlyph_.Character->getXAdvanceProperty() + font_->getLetterSpacingProperty());
        }

        if (font_->getUseKerningsProperty() && previousGlyph_.has_value() && previousGlyph_->Character)
        {
            int amount = 0;
            if (previousGlyph_->Character->getKerningsProperty().TryGetValue(character, amount))
            {
                positionDelta_.X += static_cast<float>(amount);
                currentGlyph_.Position.X += static_cast<float>(amount);
            }
        }

        previousGlyph_ = currentGlyph_;

        if (character != '\n')
        {
            return true;
        }

        positionDelta_.Y += static_cast<float>(font_->getLineHeightProperty() + font_->getLineSpacingProperty());
        // Upstream's StringBuilderGlyphEnumerator resets to `_position.X` here, while
        // StringGlyphEnumerator (the plain-string variant, above) resets to a literal `0` -- a
        // genuine inconsistency present in MonoGame.Extended itself. Preserved as-is on both sides
        // rather than "fixed" to be consistent, per this project's port-faithfully mandate.
        positionDelta_.X = position_.X;
        previousGlyph_.reset();

        return true;
    }

    BitmapFont::StringBuilderGlyphIterator& BitmapFont::StringBuilderGlyphIterator::operator++()
    {
        done_ = !MoveNext();
        return *this;
    }

    BitmapFont::StringBuilderGlyphIterator BitmapFont::StringBuilderGlyphEnumerable::begin() const
    {
        StringBuilderGlyphIterator it(font_, text_, position_);
        ++it;
        return it;
    }

    // --- FromFile / FromStream ----------------------------------------------------------------------

    BitmapFont BitmapFont::FromFile(GraphicsDevice& graphicsDevice, const std::string& path)
    {
        const std::unique_ptr<System::IO::Stream> stream = Microsoft::Xna::Framework::TitleContainer::OpenStream(path);
        return FromStream(graphicsDevice, *stream, path);
    }

    BitmapFont BitmapFont::FromStream(GraphicsDevice& graphicsDevice, System::IO::Stream& stream, const std::string& name)
    {
        const CNA::Extended::Content::BitmapFonts::BitmapFontFileContent bmfFile =
            CNA::Extended::Content::BitmapFonts::Read(stream, name);

        BitmapFont font;
        font.face_ = bmfFile.FontName;
        font.size_ = bmfFile.Info.FontSize;
        font.lineHeight_ = bmfFile.Common.LineHeight;
        font.letterSpacing_ = bmfFile.Info.SpacingHoriz;
        font.lineSpacing_ = bmfFile.Info.SpacingVert;

        // Load page textures, deduplicated by page file name (matching upstream's `pages` dictionary).
        System::Collections::Generic::Dictionary<std::string, Texture2D*> pagesByName;
        for (const std::string& pageFileName : bmfFile.Pages)
        {
            if (!pagesByName.ContainsKey(pageFileName))
            {
                const std::string texturePath = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(bmfFile.Path), pageFileName);
                const std::unique_ptr<System::IO::Stream> textureStream = Microsoft::Xna::Framework::TitleContainer::OpenStream(texturePath);
                font.pageTextures_.push_back(std::make_unique<Texture2D>(Texture2D::FromStream(graphicsDevice, *textureStream)));
                pagesByName.Add(pageFileName, font.pageTextures_.back().get());
            }
        }

        // Load characters.
        for (const Content::BitmapFonts::CharacterBlock& charBlock : bmfFile.Characters)
        {
            Texture2D* texture = pagesByName[bmfFile.Pages[charBlock.Page]];
            auto region = std::make_shared<Texture2DRegion>(texture, charBlock.X, charBlock.Y, charBlock.Width, charBlock.Height);
            auto character = std::make_shared<BitmapFontCharacter>(charBlock.ID, region, charBlock.XOffset, charBlock.YOffset, charBlock.XAdvance);
            font.characters_.Add(character->getCharacterProperty(), character);
        }

        // Load kernings.
        for (const Content::BitmapFonts::KerningPairsBlock& kerningBlock : bmfFile.Kernings)
        {
            std::shared_ptr<BitmapFontCharacter> character;
            if (font.characters_.TryGetValue(static_cast<int>(kerningBlock.First), character))
            {
                character->getKerningsProperty().Add(static_cast<int>(kerningBlock.Second), kerningBlock.Amount);
            }
        }

        return font;
    }
}
