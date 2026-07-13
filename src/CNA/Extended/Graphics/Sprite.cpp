// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/Sprite.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "System/ObjectDisposedException.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    using Microsoft::Xna::Framework::Matrix;

    namespace
    {
        void ThrowIfTextureDisposed(const std::shared_ptr<Texture2DRegion>& region)
        {
            if (region->getTextureProperty() != nullptr)
            {
                System::ObjectDisposedException::ThrowIf(region->getTextureProperty()->getIsDisposedProperty(), "textureRegion");
            }
        }
    }

    Sprite::Sprite(Texture2D* texture) : Sprite(std::make_shared<Texture2DRegion>(texture))
    {
    }

    Sprite::Sprite(const std::shared_ptr<Texture2DRegion>& textureRegion)
    {
        if (!textureRegion)
        {
            throw std::invalid_argument("textureRegion must not be null.");
        }
        ThrowIfTextureDisposed(textureRegion);

        textureRegion_ = textureRegion;
        alpha_ = 1.0f;
        color_ = Color::White;
        isVisible_ = true;
        effect_ = SpriteEffects::None;
        setOriginNormalizedProperty(textureRegion_->getOriginNormalizedProperty().value_or(Vector2::Zero));
        depth_ = 0.0f;
    }

    Sprite::Sprite(const Sprite& source)
    {
        ThrowIfTextureDisposed(source.textureRegion_);

        textureRegion_ = source.textureRegion_;
        alpha_ = source.alpha_;
        color_ = source.color_;
        isVisible_ = source.isVisible_;
        effect_ = source.effect_;
        depth_ = source.depth_;
        origin_ = source.origin_;
        tag_ = source.tag_;
    }

    Point Sprite::getSizeProperty() const
    {
        return textureRegion_->getOriginalSizeProperty();
    }

    Vector2 Sprite::getOriginNormalizedProperty() const
    {
        const Size originalSize = textureRegion_->getOriginalSizeProperty();
        return Vector2(origin_.X / static_cast<float>(originalSize.Width), origin_.Y / static_cast<float>(originalSize.Height));
    }

    void Sprite::setOriginNormalizedProperty(const Vector2& value)
    {
        const Size originalSize = textureRegion_->getOriginalSizeProperty();
        origin_ = Vector2(value.X * static_cast<float>(originalSize.Width), value.Y * static_cast<float>(originalSize.Height));
    }

    void Sprite::setTextureRegionProperty(const std::shared_ptr<Texture2DRegion>& value)
    {
        if (!value)
        {
            throw std::invalid_argument("value must not be null.");
        }
        ThrowIfTextureDisposed(value);

        textureRegion_ = value;
        if (value->getOriginNormalizedProperty().has_value())
        {
            setOriginNormalizedProperty(value->getOriginNormalizedProperty().value());
        }
    }

    RectangleF Sprite::GetBoundingRectangle(const Transform2& transform) const
    {
        return GetBoundingRectangle(transform.getPositionProperty(), transform.getRotationProperty(), transform.getScaleProperty());
    }

    RectangleF Sprite::GetBoundingRectangle(const Vector2& position, float rotation, const Vector2& scale) const
    {
        const std::array<Vector2, 4> corners = GetCorners(position, rotation, scale);
        Vector2 min = corners[0];
        Vector2 max = corners[0];
        for (const Vector2& corner : corners)
        {
            min = Vector2(std::min(min.X, corner.X), std::min(min.Y, corner.Y));
            max = Vector2(std::max(max.X, corner.X), std::max(max.Y, corner.Y));
        }
        return RectangleF(min.X, min.Y, max.X - min.X, max.Y - min.Y);
    }

    std::array<Vector2, 4> Sprite::GetCorners(const Vector2& position, float rotation, const Vector2& scale) const
    {
        const Size originalSize = textureRegion_->getOriginalSizeProperty();
        Vector2 min = -origin_;
        Vector2 max = min + Vector2(static_cast<float>(originalSize.Width), static_cast<float>(originalSize.Height));
        const Vector2 offset = position;

        if (scale != Vector2::One)
        {
            min = min * scale;
            max = max * scale;
        }

        std::array<Vector2, 4> corners{};
        corners[0] = min;
        corners[1] = Vector2(max.X, min.Y);
        corners[2] = max;
        corners[3] = Vector2(min.X, max.Y);

        if (rotation != 0)
        {
            const Matrix matrix = Matrix::CreateRotationZ(rotation);
            for (Vector2& corner : corners)
            {
                corner = Vector2::Transform(corner, matrix);
            }
        }

        for (Vector2& corner : corners)
        {
            corner = corner + offset;
        }

        return corners;
    }

    Sprite Sprite::Clone() const
    {
        return Sprite(*this);
    }
}
