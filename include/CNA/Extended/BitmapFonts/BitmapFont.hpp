// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BitmapFonts/BitmapFont.cs. `_characters` (a
// `Dictionary<int, BitmapFontCharacter>` of reference-type values in C#) becomes a
// `Dictionary<int, std::shared_ptr<BitmapFontCharacter>>` -- cheap-to-copy, nullable-lookup-result
// semantics matching upstream's reference-type dictionary values exactly.
//
// Upstream's `StringGlyphEnumerable`/`StringGlyphEnumerator` (and the StringBuilder-keyed
// equivalents) are C#'s struct-based custom `IEnumerable<T>`/`IEnumerator<T>` pattern; ported
// as real C++ input iterators exposing `begin()`/`end()` so `for (auto glyph : font.GetGlyphs(...))`
// works with range-for, preserving the same lazy, allocation-free, one-glyph-at-a-time evaluation
// as upstream (not materialized into a container up front).
//
// `FromFile`/`FromStream` load real page-texture images from disk via CNA's `GraphicsDevice`, and
// each loaded `BitmapFontCharacter::TextureRegion` (`Texture2DRegion`, holding a non-owning
// `Texture2D*`) needs those page textures to outlive the font. `BitmapFont` therefore owns them in
// `pageTextures_` (`std::vector<std::unique_ptr<Texture2D>>` for pointer stability across growth) --
// in upstream this is implicit: the page `Texture2D`s are GC-rooted transitively through the
// `Texture2DRegion`s stored inside the font's own `_characters` dictionary, so the font is already
// the real, if implicit, root keeping them alive. This makes that same relationship an explicit
// C++ ownership member instead.
#pragma once

#include "CNA/Extended/BitmapFonts/BitmapFontCharacter.hpp"
#include "CNA/Extended/RectangleF.hpp"
#include "CNA/Extended/SizeF.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/Collections/Generic/Dictionary.hpp"
#include "System/IO/Stream.hpp"
#include "System/Text/StringBuilder.hpp"

#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Included in full (not forward-declared) because BitmapFont::pageTextures_ is a
// std::vector<std::unique_ptr<Texture2D>> member -- std::unique_ptr's destructor needs Texture2D
// to be a complete type in every translation unit where BitmapFont's implicitly-generated
// destructor gets instantiated, not just BitmapFont.cpp.
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

namespace Microsoft::Xna::Framework::Graphics
{
    class GraphicsDevice;
}

namespace CNA::Extended::BitmapFonts
{
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @brief A bitmap (BMFont) font: a texture-backed character set with kerning and glyph-layout
     * support, loaded from an AngelCode BMFont `.fnt` file (plus its page texture image(s)) via
     * FromFile/FromStream. Use MeasureString/GetStringRectangle for layout, or draw text directly
     * with BitmapFontExtensions::DrawString.
     *
     * @see BitmapFontExtensions::DrawString for drawing text with this font through a SpriteBatch.
     * @see GetGlyphs for lower-level, allocation-free access to each laid-out glyph's position and
     * BitmapFontCharacter, e.g. for custom per-glyph effects.
     * @code
     * #include <CNA/Extended/BitmapFonts/BitmapFont.hpp>
     * #include <CNA/Extended/BitmapFonts/BitmapFontExtensions.hpp>
     *
     * using CNA::Extended::BitmapFonts::BitmapFont;
     * using CNA::Extended::BitmapFonts::DrawString;
     * using Microsoft::Xna::Framework::Color;
     * using Microsoft::Xna::Framework::Vector2;
     * using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
     * using Microsoft::Xna::Framework::Graphics::SpriteBatch;
     *
     * void DrawHud(GraphicsDevice& graphicsDevice, SpriteBatch& spriteBatch)
     * {
     *     const BitmapFont font = BitmapFont::FromFile(graphicsDevice, "Content/font.fnt");
     *     DrawString(spriteBatch, font, "Hello, world!", Vector2(10.0f, 10.0f), Color::White);
     * }
     * @endcode
     */
    class BitmapFont
    {
    public:
        /** @brief One positioned glyph produced while laying out a string, as returned by GetGlyphs(). */
        struct BitmapFontGlyph
        {
            int CharacterID = 0;
            Vector2 Position;
            std::shared_ptr<BitmapFontCharacter> Character;
        };

        /**
         * @brief Initializes a new BitmapFont with zero letter/line spacing.
         * @param face The font face name.
         * @param size The font size.
         * @param lineHeight The line height, in pixels.
         * @param characters The characters that make up this font.
         */
        BitmapFont(std::string face, int size, int lineHeight, const std::vector<std::shared_ptr<BitmapFontCharacter>>& characters);

        /**
         * @brief Initializes a new BitmapFont.
         * @param face The font face name.
         * @param size The font size.
         * @param lineHeight The line height, in pixels.
         * @param letterSpacing Extra horizontal spacing applied between characters.
         * @param lineSpacing Extra vertical spacing applied between lines.
         * @param characters The characters that make up this font.
         */
        BitmapFont(std::string face, int size, int lineHeight, int letterSpacing, int lineSpacing,
            const std::vector<std::shared_ptr<BitmapFontCharacter>>& characters);

        /** @return The font face name. */
        [[nodiscard]] const std::string& getFaceProperty() const { return face_; }
        /** @return The font size. */
        [[nodiscard]] int getSizeProperty() const { return size_; }
        /** @return The line height, in pixels. */
        [[nodiscard]] int getLineHeightProperty() const { return lineHeight_; }

        /** @return Extra horizontal spacing applied between characters. */
        [[nodiscard]] int getLetterSpacingProperty() const { return letterSpacing_; }
        /** @brief Sets extra horizontal spacing applied between characters. */
        void setLetterSpacingProperty(int value) { letterSpacing_ = value; }

        /** @return Extra vertical spacing applied between lines. */
        [[nodiscard]] int getLineSpacingProperty() const { return lineSpacing_; }
        /** @brief Sets extra vertical spacing applied between lines. */
        void setLineSpacingProperty(int value) { lineSpacing_ = value; }

        /** @return Whether kerning pairs are applied while laying out glyphs. Defaults to true. */
        [[nodiscard]] bool getUseKerningsProperty() const { return useKernings_; }
        /** @brief Sets whether kerning pairs are applied while laying out glyphs. */
        void setUseKerningsProperty(bool value) { useKernings_ = value; }

        /** @return The character with the given code, or nullptr if this font has no glyph for it. */
        [[nodiscard]] std::shared_ptr<BitmapFontCharacter> GetCharacter(int character) const;
        /** @return true if this font has a glyph for @p character; @p value receives it (or nullptr). */
        bool TryGetCharacter(int character, std::shared_ptr<BitmapFontCharacter>& value) const;

        /** @return The size of @p text when laid out with this font, or SizeF::Empty for an empty string. */
        [[nodiscard]] SizeF MeasureString(const std::string& text) const;
        /** @return The size of @p text when laid out with this font, or SizeF::Empty for an empty/null builder. */
        [[nodiscard]] SizeF MeasureString(const System::Text::StringBuilder* text) const;

        /** @return The bounding rectangle of @p text laid out at the origin. */
        [[nodiscard]] RectangleF GetStringRectangle(const std::string& text) const;
        /** @return The bounding rectangle of @p text laid out at @p position. */
        [[nodiscard]] RectangleF GetStringRectangle(const std::string& text, const Vector2& position) const;
        /** @return The bounding rectangle of @p text laid out at @p position (defaults to the origin). */
        [[nodiscard]] RectangleF GetStringRectangle(const System::Text::StringBuilder* text, const std::optional<Vector2>& position = std::nullopt) const;

        /** @return A human-readable description of this font. */
        [[nodiscard]] std::string ToString() const { return face_; }

        /** @brief Loads a BitmapFont from a BMFont file at @p path (via TitleContainer). */
        [[nodiscard]] static BitmapFont FromFile(GraphicsDevice& graphicsDevice, const std::string& path);
        /** @brief Loads a BitmapFont from an open stream containing BMFont file content. */
        [[nodiscard]] static BitmapFont FromStream(GraphicsDevice& graphicsDevice, System::IO::Stream& stream, const std::string& name);

    public:
        class StringGlyphEnumerable;
        class StringBuilderGlyphEnumerable;

        /** @brief Lazily lays out @p text's glyphs, starting at @p position (defaults to the origin). Supports range-for. */
        [[nodiscard]] StringGlyphEnumerable GetGlyphs(const std::string& text, const std::optional<Vector2>& position = std::nullopt) const;
        /** @brief Lazily lays out @p text's glyphs, starting at @p position (defaults to the origin). Supports range-for. */
        [[nodiscard]] StringBuilderGlyphEnumerable GetGlyphs(const System::Text::StringBuilder* text, const std::optional<Vector2>& position) const;

        // --- Iterator/enumerable definitions (public so GetGlyphs()'s return types are usable by callers) ---

        class StringGlyphIterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = BitmapFontGlyph;
            using difference_type = std::ptrdiff_t;
            using pointer = const BitmapFontGlyph*;
            using reference = const BitmapFontGlyph&;

            StringGlyphIterator() = default; // end sentinel
            StringGlyphIterator(const BitmapFont* font, const std::string* text, const Vector2& position);

            [[nodiscard]] const BitmapFontGlyph& operator*() const { return currentGlyph_; }
            [[nodiscard]] const BitmapFontGlyph* operator->() const { return &currentGlyph_; }
            StringGlyphIterator& operator++();
            [[nodiscard]] bool operator==(const StringGlyphIterator& other) const { return done_ == other.done_; }
            [[nodiscard]] bool operator!=(const StringGlyphIterator& other) const { return !(*this == other); }

        private:
            bool MoveNext();

            const BitmapFont* font_ = nullptr;
            const std::string* text_ = nullptr;
            std::size_t index_ = 0;
            Vector2 position_;
            Vector2 positionDelta_;
            BitmapFontGlyph currentGlyph_;
            std::optional<BitmapFontGlyph> previousGlyph_;
            bool done_ = true;
        };

        class StringGlyphEnumerable
        {
        public:
            StringGlyphEnumerable(const BitmapFont* font, const std::string* text, const Vector2& position)
                : font_(font), text_(text), position_(position)
            {
            }

            [[nodiscard]] StringGlyphIterator begin() const;
            [[nodiscard]] StringGlyphIterator end() const { return StringGlyphIterator(); }

        private:
            const BitmapFont* font_;
            const std::string* text_;
            Vector2 position_;
        };

        class StringBuilderGlyphIterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = BitmapFontGlyph;
            using difference_type = std::ptrdiff_t;
            using pointer = const BitmapFontGlyph*;
            using reference = const BitmapFontGlyph&;

            StringBuilderGlyphIterator() = default; // end sentinel
            StringBuilderGlyphIterator(const BitmapFont* font, const System::Text::StringBuilder* text, const Vector2& position);

            [[nodiscard]] const BitmapFontGlyph& operator*() const { return currentGlyph_; }
            [[nodiscard]] const BitmapFontGlyph* operator->() const { return &currentGlyph_; }
            StringBuilderGlyphIterator& operator++();
            [[nodiscard]] bool operator==(const StringBuilderGlyphIterator& other) const { return done_ == other.done_; }
            [[nodiscard]] bool operator!=(const StringBuilderGlyphIterator& other) const { return !(*this == other); }

        private:
            bool MoveNext();

            const BitmapFont* font_ = nullptr;
            const System::Text::StringBuilder* text_ = nullptr;
            std::size_t index_ = 0;
            Vector2 position_;
            Vector2 positionDelta_;
            BitmapFontGlyph currentGlyph_;
            std::optional<BitmapFontGlyph> previousGlyph_;
            bool done_ = true;
        };

        class StringBuilderGlyphEnumerable
        {
        public:
            StringBuilderGlyphEnumerable(const BitmapFont* font, const System::Text::StringBuilder* text, const Vector2& position)
                : font_(font), text_(text), position_(position)
            {
            }

            [[nodiscard]] StringBuilderGlyphIterator begin() const;
            [[nodiscard]] StringBuilderGlyphIterator end() const { return StringBuilderGlyphIterator(); }

        private:
            const BitmapFont* font_;
            const System::Text::StringBuilder* text_;
            Vector2 position_;
        };

    private:
        // Used only by FromStream(), which builds up a font's fields incrementally while loading
        // page textures and characters from a parsed BMFont file.
        BitmapFont() = default;

        std::string face_;
        int size_ = 0;
        int lineHeight_ = 0;
        int letterSpacing_ = 0;
        int lineSpacing_ = 0;
        bool useKernings_ = true;
        System::Collections::Generic::Dictionary<int, std::shared_ptr<BitmapFontCharacter>> characters_;

        // Owns page textures loaded by FromFile/FromStream so the Texture2D* inside each loaded
        // character's Texture2DRegion stays valid for the font's lifetime -- see this file's header
        // comment.
        std::vector<std::unique_ptr<Texture2D>> pageTextures_;
    };
}
