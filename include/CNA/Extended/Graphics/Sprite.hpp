// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Graphics/Sprite.cs. `Texture2DRegion TextureRegion` is a
// std::shared_ptr<Texture2DRegion>, matching Texture2DRegion.hpp's shared-ownership convention
// (a Sprite constructed directly from a Texture2D owns the region it creates; a Sprite
// constructed from an atlas-owned or otherwise externally-owned region shares that ownership --
// exactly the aliasing C#'s GC reference semantics give upstream "for free"). `object Tag` ->
// `System::Object*`, matching GraphicsResource::getTagProperty()'s established convention for
// this exact C# shape elsewhere in this ecosystem. `Vector2[] GetCorners(...)` (always length 4)
// -> `std::array<Vector2, 4>`. Sprite has no virtual members in upstream (it is not `sealed` but
// none of its members are declared `virtual`); `AnimatedSprite` extends it without overriding
// anything, so no vtable is strictly required -- a virtual destructor is added anyway as ordinary
// C++ hygiene for a base class with a real derived class.
#pragma once

#include "CNA/Extended/Graphics/Texture2DRegion.hpp"
#include "CNA/Extended/IColorable.hpp"
#include "CNA/Extended/RectangleF.hpp"
#include "CNA/Extended/Transform.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Point.hpp"
#include "System/Object.hpp"

#include <array>
#include <memory>

namespace Microsoft::Xna::Framework::Graphics
{
    class Texture2D;
}

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    /**
     * @brief Represents a drawable 2D texture region with additional properties for rendering,
     * such as position, scale, rotation, and color.
     */
    class Sprite : public IColorable
    {
    public:
        /**
         * @brief Initializes a sprite representing the entire texture.
         * @throws std::invalid_argument @p texture is null.
         * @throws System::ObjectDisposedException @p texture was already disposed.
         */
        explicit Sprite(Texture2D* texture);

        /**
         * @brief Initializes a sprite from an existing texture region (shared; see this file's
         * header comment).
         * @throws std::invalid_argument @p textureRegion is null.
         * @throws System::ObjectDisposedException the region's source texture was already disposed.
         */
        explicit Sprite(const std::shared_ptr<Texture2DRegion>& textureRegion);

        /**
         * @brief Copy-constructs a shallow copy: the new sprite shares the same TextureRegion
         * reference; Tag is copied by reference, not cloned.
         * @throws System::ObjectDisposedException @p source's texture region's source texture was already disposed.
         */
        Sprite(const Sprite& source);

        Sprite& operator=(const Sprite&) = default;
        Sprite(Sprite&&) = default;
        Sprite& operator=(Sprite&&) = default;

        ~Sprite() override = default;

        /** @brief Gets whether this sprite is visible. */
        [[nodiscard]] bool getIsVisibleProperty() const { return isVisible_; }
        /** @brief Sets whether this sprite is visible. */
        void setIsVisibleProperty(bool value) { isVisible_ = value; }

        /** @brief Gets the color mask used when rendering this sprite. */
        [[nodiscard]] Color getColorProperty() const override { return color_; }
        /** @brief Sets the color mask used when rendering this sprite. */
        void setColorProperty(const Color& value) override { color_ = value; }

        /** @brief Gets the alpha transparency (0 = fully transparent, 1 = fully opaque) used when rendering this sprite. */
        [[nodiscard]] float getAlphaProperty() const { return alpha_; }
        /** @brief Sets the alpha transparency used when rendering this sprite. */
        void setAlphaProperty(float value) { alpha_ = value; }

        /** @brief Gets the layer depth used when rendering this sprite (higher values render on top). */
        [[nodiscard]] float getDepthProperty() const { return depth_; }
        /** @brief Sets the layer depth used when rendering this sprite. */
        void setDepthProperty(float value) { depth_ = value; }

        /** @brief Gets the sprite effects (flip flags) to apply when rendering this sprite. */
        [[nodiscard]] SpriteEffects getEffectProperty() const { return effect_; }
        /** @brief Sets the sprite effects to apply when rendering this sprite. */
        void setEffectProperty(SpriteEffects value) { effect_ = value; }

        /** @brief Gets the user-defined data associated with this sprite. */
        [[nodiscard]] System::Object* getTagProperty() const { return tag_; }
        /** @brief Sets the user-defined data associated with this sprite. */
        void setTagProperty(System::Object* value) { tag_ = value; }

        /** @brief Gets the size of the sprite (its texture region's original size). */
        [[nodiscard]] Point getSizeProperty() const;

        /** @brief Gets the origin, relative to the texture region's bounds, around which the sprite is rotated and scaled. */
        [[nodiscard]] Vector2 getOriginProperty() const { return origin_; }
        /** @brief Sets the origin around which the sprite is rotated and scaled. */
        void setOriginProperty(const Vector2& value) { origin_ = value; }

        /** @brief Gets the origin as a fraction of the texture region's extents ((0,0) top-left, (1,1) bottom-right). */
        [[nodiscard]] Vector2 getOriginNormalizedProperty() const;
        /** @brief Sets the origin as a fraction of the texture region's extents. */
        void setOriginNormalizedProperty(const Vector2& value);

        /**
         * @brief Gets the source texture region of this sprite.
         */
        [[nodiscard]] const std::shared_ptr<Texture2DRegion>& getTextureRegionProperty() const { return textureRegion_; }

        /**
         * @brief Sets the source texture region of this sprite.
         * @throws std::invalid_argument @p value is null.
         * @throws System::ObjectDisposedException the region's source texture was already disposed.
         */
        void setTextureRegionProperty(const std::shared_ptr<Texture2DRegion>& value);

        /** @brief Gets the bounding rectangle of the sprite in world/screen coordinates. */
        [[nodiscard]] RectangleF GetBoundingRectangle(const Transform2& transform) const;

        /** @brief Gets the bounding rectangle of the sprite in world/screen coordinates. */
        [[nodiscard]] RectangleF GetBoundingRectangle(const Vector2& position, float rotation, const Vector2& scale) const;

        /** @brief Gets the corner points of the sprite in world/screen coordinates, in top-left/top-right/bottom-right/bottom-left order. */
        [[nodiscard]] std::array<Vector2, 4> GetCorners(const Vector2& position, float rotation, const Vector2& scale) const;

        /**
         * @brief Creates a shallow copy of this sprite: the returned sprite shares the same
         * TextureRegion reference; Tag is copied by reference, not cloned.
         * @throws System::ObjectDisposedException this sprite's texture region's source texture was already disposed.
         */
        [[nodiscard]] Sprite Clone() const;

    private:
        std::shared_ptr<Texture2DRegion> textureRegion_;
        bool isVisible_ = true;
        Color color_ = Color::White;
        float alpha_ = 1.0f;
        float depth_ = 0.0f;
        SpriteEffects effect_ = SpriteEffects::None;
        System::Object* tag_ = nullptr;
        Vector2 origin_;
    };
}
