// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/Texture2DRegion.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/ObjectDisposedException.hpp"

#include <stdexcept>

namespace CNA::Extended::Graphics
{
    Texture2DRegion::Texture2DRegion(Texture2D* texture)
        : Texture2DRegion(texture, 0, 0, texture != nullptr ? texture->getWidthProperty() : 0,
              texture != nullptr ? texture->getHeightProperty() : 0, "")
    {
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, const std::string& /*name*/)
        : Texture2DRegion(texture, 0, 0, texture != nullptr ? texture->getBoundsProperty().Width : 0,
              texture != nullptr ? texture->getBoundsProperty().Height : 0, "")
    {
        // Upstream's Texture2DRegion(Texture2D, string) ignores its `name` parameter and always
        // delegates with null -- see this type's header comment.
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, const Rectangle& region)
        : Texture2DRegion(texture, region.X, region.Y, region.Width, region.Height, "")
    {
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, int x, int y, int width, int height)
        : Texture2DRegion(texture, x, y, width, height, "")
    {
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, const Rectangle& region, const std::string& name)
        : Texture2DRegion(texture, region.X, region.Y, region.Width, region.Height, name)
    {
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, int x, int y, int width, int height, const std::string& name)
        : Texture2DRegion(texture, x, y, width, height, false, Size(width, height), Vector2::Zero, std::nullopt, name)
    {
    }

    Texture2DRegion::Texture2DRegion(Texture2D* texture, int x, int y, int width, int height, bool isRotated,
        const Size& originalSize, const Vector2& offset, const std::optional<Vector2>& originNormalized,
        const std::string& name)
        : x_(x), y_(y), width_(width), height_(height), size_(width, height), bounds_(x, y, width, height),
          isRotated_(isRotated), originalSize_(originalSize), offset_(offset), originNormalized_(originNormalized)
    {
        if (texture == nullptr)
        {
            throw std::invalid_argument("texture must not be null.");
        }
        System::ObjectDisposedException::ThrowIf(texture->getIsDisposedProperty(), "texture");

        name_ = name.empty() ? texture->getNameProperty() : name;
        texture_ = texture;
        topUV_ = static_cast<float>(bounds_.getTopProperty()) / static_cast<float>(texture->getHeightProperty());
        rightUV_ = static_cast<float>(bounds_.getRightProperty()) / static_cast<float>(texture->getWidthProperty());
        bottomUV_ = static_cast<float>(bounds_.getBottomProperty()) / static_cast<float>(texture->getHeightProperty());
        leftUV_ = static_cast<float>(bounds_.getLeftProperty()) / static_cast<float>(texture->getWidthProperty());
    }

    Texture2DRegion::Texture2DRegion(const std::string& name, const Rectangle& bounds)
        : Texture2DRegion(name, bounds.X, bounds.Y, bounds.Width, bounds.Height)
    {
    }

    Texture2DRegion::Texture2DRegion(const std::string& name, int x, int y, int width, int height)
        : name_(name), x_(x), y_(y), width_(width), height_(height), size_(width, height),
          bounds_(x, y, width, height), originalSize_(width, height)
    {
        topUV_ = static_cast<float>(bounds_.getTopProperty()) / 1.0f;
        rightUV_ = static_cast<float>(bounds_.getRightProperty()) / 1.0f;
        bottomUV_ = static_cast<float>(bounds_.getBottomProperty()) / 1.0f;
        leftUV_ = static_cast<float>(bounds_.getLeftProperty()) / 1.0f;
    }

    std::string Texture2DRegion::ToString() const
    {
        return name_ + " " + bounds_.ToString();
    }
}
