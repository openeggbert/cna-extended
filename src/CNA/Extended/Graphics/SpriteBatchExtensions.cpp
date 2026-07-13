// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/SpriteBatchExtensions.hpp"

#include "CNA/Extended/Graphics/Texture2DRegionExtensions.hpp"
#include "CNA/Extended/RectangleExtensions.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <array>
#include <numbers>
#include <utility>

namespace CNA::Extended::Graphics
{
    namespace
    {
        // Upstream: private static readonly Rectangle[] _patchCache (a shared scratch buffer
        // reused across Draw(NinePatch) calls to avoid allocating).
        std::array<Rectangle, 9> patchCache{};

        void CreateDestinationPatches(const NinePatch& ninePatch, const Rectangle& destinationRect)
        {
            const int x = destinationRect.X;
            const int y = destinationRect.Y;
            const int width = destinationRect.Width;
            const int height = destinationRect.Height;

            const Thickness& padding = ninePatch.getPaddingProperty();
            const int topPadding = padding.getTopProperty();
            const int rightPadding = padding.getRightProperty();
            const int bottomPadding = padding.getBottomProperty();
            const int leftPadding = padding.getLeftProperty();

            const int midWidth = width - leftPadding - rightPadding;
            const int midHeight = height - topPadding - bottomPadding;
            const int top = y + topPadding;
            const int right = x + width - rightPadding;
            const int bottom = y + height - bottomPadding;
            const int left = x + leftPadding;

            patchCache[NinePatch::TopLeft] = Rectangle(x, y, leftPadding, topPadding);
            patchCache[NinePatch::TopMiddle] = Rectangle(left, y, midWidth, topPadding);
            patchCache[NinePatch::TopRight] = Rectangle(right, y, rightPadding, topPadding);
            patchCache[NinePatch::MiddleLeft] = Rectangle(x, top, leftPadding, midHeight);
            patchCache[NinePatch::Middle] = Rectangle(left, top, midWidth, midHeight);
            patchCache[NinePatch::MiddleRight] = Rectangle(right, top, rightPadding, midHeight);
            patchCache[NinePatch::BottomLeft] = Rectangle(x, bottom, leftPadding, bottomPadding);
            patchCache[NinePatch::BottomMiddle] = Rectangle(left, bottom, midWidth, bottomPadding);
            patchCache[NinePatch::BottomRight] = Rectangle(right, bottom, rightPadding, bottomPadding);
        }

        bool ClipRectangles(Rectangle& sourceRectangle, Rectangle& destinationRectangle,
            const std::optional<Rectangle>& clippingRectangle, bool rotatedSource = false)
        {
            if (!clippingRectangle.has_value())
            {
                return true;
            }

            const Rectangle originalDestination = destinationRectangle;
            destinationRectangle = CNA::Extended::Clip(destinationRectangle, clippingRectangle.value());

            if (destinationRectangle == Rectangle::Empty)
            {
                return false;
            }

            const int leftDiff = destinationRectangle.getLeftProperty() - originalDestination.getLeftProperty();
            const int topDiff = destinationRectangle.getTopProperty() - originalDestination.getTopProperty();
            const int bottomDiff = originalDestination.getBottomProperty() - destinationRectangle.getBottomProperty();

            if (rotatedSource)
            {
                const float scaleX = static_cast<float>(sourceRectangle.Height) / static_cast<float>(originalDestination.Width);
                const float scaleY = static_cast<float>(sourceRectangle.Width) / static_cast<float>(originalDestination.Height);

                sourceRectangle.X += static_cast<int>(static_cast<float>(bottomDiff) * scaleY);
                sourceRectangle.Y += static_cast<int>(static_cast<float>(leftDiff) * scaleX);
                sourceRectangle.Width = static_cast<int>(static_cast<float>(destinationRectangle.Height) * scaleY);
                sourceRectangle.Height = static_cast<int>(static_cast<float>(destinationRectangle.Width) * scaleX);
            }
            else
            {
                const float scaleX = static_cast<float>(sourceRectangle.Width) / static_cast<float>(originalDestination.Width);
                const float scaleY = static_cast<float>(sourceRectangle.Height) / static_cast<float>(originalDestination.Height);

                sourceRectangle.X += static_cast<int>(static_cast<float>(leftDiff) * scaleX);
                sourceRectangle.Y += static_cast<int>(static_cast<float>(topDiff) * scaleY);
                sourceRectangle.Width = static_cast<int>(static_cast<float>(destinationRectangle.Width) * scaleX);
                sourceRectangle.Height = static_cast<int>(static_cast<float>(destinationRectangle.Height) * scaleY);
            }

            return true;
        }

        std::shared_ptr<Texture2DRegion> ClipSourceRegion(
            const std::shared_ptr<Texture2DRegion>& sourceRegion, const Rectangle& destinationRectangle, const Rectangle& clippingRectangle)
        {
            const float left = static_cast<float>(clippingRectangle.getLeftProperty() - destinationRectangle.getLeftProperty());
            const float right = static_cast<float>(destinationRectangle.getRightProperty() - clippingRectangle.getRightProperty());
            const float top = static_cast<float>(clippingRectangle.getTopProperty() - destinationRectangle.getTopProperty());
            const float bottom = static_cast<float>(destinationRectangle.getBottomProperty() - clippingRectangle.getBottomProperty());
            float x = left > 0 ? left : 0.0f;
            float y = top > 0 ? top : 0.0f;
            float w = (right > 0 ? right : 0.0f) + x;
            float h = (bottom > 0 ? bottom : 0.0f) + y;

            const Size originalSize = sourceRegion->getOriginalSizeProperty();
            const float scaleX = static_cast<float>(destinationRectangle.Width) / static_cast<float>(originalSize.Width);
            const float scaleY = static_cast<float>(destinationRectangle.Height) / static_cast<float>(originalSize.Height);
            x /= scaleX;
            y /= scaleY;
            w /= scaleX;
            h /= scaleY;

            return GetSubregion(sourceRegion, static_cast<int>(x), static_cast<int>(y),
                static_cast<int>(static_cast<float>(originalSize.Width) - w), static_cast<int>(static_cast<float>(originalSize.Height) - h));
        }

        Rectangle ClipDestinationRectangle(const Rectangle& destinationRectangle, const Rectangle& clippingRectangle)
        {
            return CNA::Extended::Clip(destinationRectangle, clippingRectangle);
        }
    }

    void Draw(SpriteBatch& spriteBatch, const NinePatch& ninePatchRegion, const Rectangle& destinationRectangle,
        const Color& color, const std::optional<Rectangle>& clippingRectangle)
    {
        CreateDestinationPatches(ninePatchRegion, destinationRectangle);
        const std::vector<std::shared_ptr<Texture2DRegion>>& sourcePatches = ninePatchRegion.getPatchesProperty();

        for (std::size_t i = 0; i < sourcePatches.size(); ++i)
        {
            std::shared_ptr<Texture2DRegion> sourceRegion = sourcePatches[i];
            Rectangle destinationRect = patchCache[i];

            if (clippingRectangle.has_value())
            {
                sourceRegion = ClipSourceRegion(sourceRegion, destinationRect, clippingRectangle.value());
                destinationRect = ClipDestinationRectangle(destinationRect, clippingRectangle.value());
            }
            if (sourceRegion && !destinationRect.getIsEmptyProperty())
            {
                Draw(spriteBatch, sourceRegion, destinationRect, color);
            }
        }
    }

    void Draw(const Sprite& sprite, SpriteBatch& spriteBatch, const Vector2& position, float rotation, const Vector2& scale)
    {
        Draw(spriteBatch, sprite, position, rotation, scale);
    }

    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Transform2& transform)
    {
        Draw(spriteBatch, sprite, transform.getPositionProperty(), transform.getRotationProperty(), transform.getScaleProperty());
    }

    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Vector2& position, float rotation)
    {
        Draw(spriteBatch, sprite, position, rotation, Vector2::One);
    }

    void Draw(SpriteBatch& spriteBatch, const Sprite& sprite, const Vector2& position, float rotation, const Vector2& scale)
    {
        if (sprite.getIsVisibleProperty())
        {
            Draw(spriteBatch, sprite.getTextureRegionProperty(), position, sprite.getColorProperty() * sprite.getAlphaProperty(),
                rotation, sprite.getOriginProperty(), scale, sprite.getEffectProperty(), sprite.getDepthProperty());
        }
    }

    void Draw(SpriteBatch& spriteBatch, Texture2D& texture, Rectangle sourceRectangle, Rectangle destinationRectangle,
        const Color& color, const std::optional<Rectangle>& clippingRectangle)
    {
        if (!ClipRectangles(sourceRectangle, destinationRectangle, clippingRectangle))
        {
            return;
        }

        if (destinationRectangle.Width > 0 && destinationRectangle.Height > 0)
        {
            spriteBatch.Draw(texture, destinationRectangle, sourceRectangle, color);
        }
    }

    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion, const Vector2& position,
        const Color& color, const std::optional<Rectangle>& clippingRectangle)
    {
        Draw(spriteBatch, textureRegion, position, color, 0.0f, Vector2::Zero, Vector2::One, SpriteEffects::None, 0.0f, clippingRectangle);
    }

    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion, const Vector2& position,
        const Color& color, float rotation, const Vector2& origin, const Vector2& scale, SpriteEffects effects,
        float layerDepth, const std::optional<Rectangle>& clippingRectangle)
    {
        Rectangle sourceRectangle = textureRegion->getBoundsProperty();
        Vector2 offset = origin - textureRegion->getOffsetProperty();
        Vector2 sourceScale = scale;

        // Handle rotated texture regions
        if (textureRegion->getIsRotatedProperty())
        {
            const Vector2 rotatedOrigin(origin.Y, -origin.X);
            const Vector2 rotatedTrimOffset(textureRegion->getOffsetProperty().Y, -textureRegion->getOffsetProperty().X);
            const Vector2 shiftByWidth(static_cast<float>(textureRegion->getSizeProperty().Width), 0.0f);
            offset = rotatedTrimOffset - rotatedOrigin + shiftByWidth;

            // Swap scale axes and adjust rotation for rotated regions
            sourceScale = Vector2(scale.Y, scale.X);
            rotation -= std::numbers::pi_v<float> / 2.0f;

            switch (effects)
            {
                case SpriteEffects::FlipHorizontally:
                    effects = SpriteEffects::FlipVertically;
                    break;
                case SpriteEffects::FlipVertically:
                    effects = SpriteEffects::FlipHorizontally;
                    break;
                default:
                    break; // nothing to do if flipped in both directions
            }
        }

        if (clippingRectangle.has_value())
        {
            const float scaledOffsetX = (origin.X - textureRegion->getOffsetProperty().X) * scale.X;
            const float scaledOffsetY = (origin.Y - textureRegion->getOffsetProperty().Y) * scale.Y;

            const int x = static_cast<int>(position.X - scaledOffsetX);
            const int y = static_cast<int>(position.Y - scaledOffsetY);

            int width = static_cast<int>(static_cast<float>(textureRegion->getWidthProperty()) * sourceScale.X);
            int height = static_cast<int>(static_cast<float>(textureRegion->getHeightProperty()) * sourceScale.Y);
            if (textureRegion->getIsRotatedProperty())
            {
                std::swap(width, height);
            }
            Rectangle destinationRectangle(x, y, width, height);

            if (!ClipRectangles(sourceRectangle, destinationRectangle, clippingRectangle, textureRegion->getIsRotatedProperty()))
            {
                // Clipped rectangle is empty, nothing to draw
                return;
            }

            if (textureRegion->getIsRotatedProperty())
            {
                offset.X -= static_cast<float>(y + height - destinationRectangle.getBottomProperty()) / sourceScale.X;
                offset.Y += (position.X - (static_cast<float>(destinationRectangle.X) + scaledOffsetX)) / sourceScale.Y;
            }
            else
            {
                offset.X += (position.X - (static_cast<float>(destinationRectangle.X) + scaledOffsetX)) / sourceScale.X;
                offset.Y += (position.Y - (static_cast<float>(destinationRectangle.Y) + scaledOffsetY)) / sourceScale.Y;
            }
        }

        spriteBatch.Draw(*textureRegion->getTextureProperty(), position, std::optional<Rectangle>(sourceRectangle), color,
            rotation, offset, sourceScale, effects, layerDepth);
    }

    void Draw(SpriteBatch& spriteBatch, const std::shared_ptr<Texture2DRegion>& textureRegion, const Rectangle& destinationRectangle,
        const Color& color, const std::optional<Rectangle>& clippingRectangle)
    {
        const Size originalSize = textureRegion->getOriginalSizeProperty();
        const float scaleX = static_cast<float>(destinationRectangle.Width) / static_cast<float>(originalSize.Width);
        const float scaleY = static_cast<float>(destinationRectangle.Height) / static_cast<float>(originalSize.Height);
        Draw(spriteBatch, textureRegion, Vector2(static_cast<float>(destinationRectangle.X), static_cast<float>(destinationRectangle.Y)),
            color, 0.0f, Vector2::Zero, Vector2(scaleX, scaleY), SpriteEffects::None, 0.0f, clippingRectangle);
    }
}
