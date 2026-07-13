// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's BitmapFonts/BitmapFont.Extensions.cs (upstream static class
// `BitmapFontExtensions`). Extension methods -> free functions in this namespace, matching this
// project's established convention (see Graphics/SpriteBatchExtensions.hpp). `SpriteEffects`
// other than `None` are unsupported for BitmapFont drawing upstream (throws
// NotSupportedException) -- preserved as the same throw here.
#pragma once

#include "CNA/Extended/BitmapFonts/BitmapFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "System/Text/StringBuilder.hpp"

#include <optional>

namespace CNA::Extended::BitmapFonts
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;

    /** @brief Draws @p text with @p bitmapFont, with full control over rotation, origin, scale, effects and layer. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& bitmapFont, const std::string& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p bitmapFont, with full control over rotation, origin, scale, effects and layer. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& bitmapFont, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p font, with full control over rotation, origin, uniform scale, effects and layer. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        float rotation, const Vector2& origin, float scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p font, with full control over rotation, origin, uniform scale, effects and layer. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, float scale, SpriteEffects effect, float layerDepth,
        const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p font at @p position, tinted by @p color, on layer @p layerDepth. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        float layerDepth, const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p font at @p position, tinted by @p color, on layer 0. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const std::string& text, const Vector2& position, const Color& color,
        const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws @p text with @p font at @p position, tinted by @p color, on layer 0. */
    void DrawString(SpriteBatch& spriteBatch, const BitmapFont& font, const System::Text::StringBuilder& text, const Vector2& position,
        const Color& color, const std::optional<Rectangle>& clippingRectangle = std::nullopt);
}
