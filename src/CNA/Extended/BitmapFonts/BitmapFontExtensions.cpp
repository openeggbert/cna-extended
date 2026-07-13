// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BitmapFonts/BitmapFontExtensions.hpp"

#include "CNA/Extended/Graphics/SpriteBatchExtensions.hpp"
#include "System/NotSupportedException.hpp"

namespace CNA::Extended::BitmapFonts
{
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& bitmapFont, const std::string& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle)
    {
        if (effect != SpriteEffects::None)
        {
            throw System::NotSupportedException("SpriteEffects other than None are not currently supported for BitmapFont");
        }

        for (const BitmapFont::BitmapFontGlyph& glyph : bitmapFont.GetGlyphs(text, position))
        {
            if (!glyph.Character)
            {
                continue;
            }
            const Vector2 characterOrigin = position - glyph.Position + origin;
            CNA::Extended::Graphics::Draw(spriteBatch, glyph.Character->getTextureRegionProperty(), position, color, rotation,
                characterOrigin, scale, effect, layerDepth, clippingRectangle);
        }
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& bitmapFont, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle)
    {
        if (effect != SpriteEffects::None)
        {
            throw System::NotSupportedException("SpriteEffects other than None are not currently supported for BitmapFont");
        }

        for (const BitmapFont::BitmapFontGlyph& glyph : bitmapFont.GetGlyphs(&text, position))
        {
            if (!glyph.Character)
            {
                continue;
            }
            const Vector2 characterOrigin = position - glyph.Position + origin;
            CNA::Extended::Graphics::Draw(spriteBatch, glyph.Character->getTextureRegionProperty(), position, color, rotation,
                characterOrigin, scale, effect, layerDepth, clippingRectangle);
        }
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        float rotation, const Vector2& origin, float scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle)
    {
        DrawString(spriteBatch, font, text, position, color, rotation, origin, Vector2(scale, scale), effect, layerDepth, clippingRectangle);
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, float scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle)
    {
        DrawString(spriteBatch, font, text, position, color, rotation, origin, Vector2(scale, scale), effect, layerDepth, clippingRectangle);
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        float layerDepth, const std::optional<Rectangle>& clippingRectangle)
    {
        DrawString(spriteBatch, font, text, position, color, 0.0f, Vector2(), Vector2(1.0f, 1.0f), SpriteEffects::None, layerDepth,
            clippingRectangle);
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        const std::optional<Rectangle>& clippingRectangle)
    {
        DrawString(spriteBatch, font, text, position, color, 0.0f, Vector2(), Vector2(1.0f, 1.0f), SpriteEffects::None, 0.0f,
            clippingRectangle);
    }

    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, const std::optional<Rectangle>& clippingRectangle)
    {
        DrawString(spriteBatch, font, text, position, color, 0.0f, Vector2(), Vector2(1.0f, 1.0f), SpriteEffects::None, 0.0f,
            clippingRectangle);
    }
}
