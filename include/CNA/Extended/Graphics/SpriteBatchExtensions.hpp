// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/SpriteBatch.Extensions.cs (upstream static class
// `SpriteBatchExtensions`). Extension methods -> free functions in this namespace, matching this
// project's established convention. `Sprite`/`SpriteBatch` receivers are taken by reference
// (non-null by construction in C++); upstream's one explicit `if (sprite == null) throw
// ArgumentNullException` check is therefore dropped, matching ActorPairKey.hpp's precedent for
// reference-type parameters. `Texture2DRegion` parameters are `const
// std::shared_ptr<Texture2DRegion>&`, matching Texture2DRegion.hpp's shared-ownership convention.
// Upstream's `private static readonly Rectangle[] _patchCache` (a shared scratch buffer reused
// across Draw(NinePatch) calls to avoid allocating) is kept as a translation-unit-local static
// array in the .cpp, matching its role as a private implementation-detail cache rather than public
// API; upstream's `private static Rectangle _rect = default` is dead code (declared, never read
// or written anywhere in SpriteBatch.Extensions.cs) and is not ported.
#pragma once

#include "CNA/Extended/Graphics/NinePatch.hpp"
#include "CNA/Extended/Graphics/Sprite.hpp"
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/Transform.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"

#include <memory>
#include <optional>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @brief Draws a nine-patch region, stretching its edge/center patches to fill @p destinationRectangle.
     * @see Graphics::NinePatch, whose corner/edge/center-patch layout this scales into @p destinationRectangle.
     */
    void Draw(SpriteBatch& spriteBatch, const NinePatch& ninePatchRegion, const Rectangle& destinationRectangle,
        const Color& color, const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /**
     * @brief Draws a sprite with explicit position, rotation, and scale (Sprite-first parameter
     * order, matching upstream's `sprite.Draw(spriteBatch, ...)` extension-method call site --
     * functionally identical to the SpriteBatch-first overload with the same three parameters below).
     */
    void Draw(const Sprite& sprite, SpriteBatch& spriteBatch, const Vector2& position, float rotation, const Vector2& scale);

    /** @brief Draws a sprite using position/rotation/scale packed into a single Transform2. */
    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Transform2& transform);

    /**
     * @brief Draws a sprite at @p position with an optional rotation, using the sprite's own
     * Scale/Origin/Color/Effects/LayerDepth (see Sprite's properties).
     * @see Graphics::AnimatedSprite, whose frame-advancing TextureRegion this overload (and its
     * siblings above/below) draws just like any other Sprite.
     */
    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Vector2& position, float rotation = 0.0f);

    /**
     * @brief Draws a sprite with explicit position, rotation, and scale (SpriteBatch-first
     * parameter order; see the Sprite-first overload above for the equivalent call written the
     * other way).
     */
    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Vector2& position, float rotation, const Vector2& scale);

    /** @brief Draws a region of a texture into a destination rectangle with a tint color and optional clipping. */
    void Draw(SpriteBatch& spriteBatch, Texture2D& texture, Rectangle sourceRectangle, Rectangle destinationRectangle,
        const Color& color, const std::optional<Rectangle>& clippingRectangle);

    /** @brief Draws a texture region to the sprite batch. */
    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion, const Vector2& position,
        const Color& color, const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws a texture region to the sprite batch with rotation, origin, scale, effects, and depth. */
    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effects,
        float layerDepth, const std::optional<Rectangle>& clippingRectangle = std::nullopt);

    /** @brief Draws a texture region into a destination rectangle, scaling it to fit. */
    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion,
        const Rectangle& destinationRectangle, const Color& color, const std::optional<Rectangle>& clippingRectangle = std::nullopt);
}
