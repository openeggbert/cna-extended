// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Graphics/Texture2DRegionExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"

#include <algorithm>
#include <stdexcept>

namespace CNA::Extended::Graphics
{
    std::shared_ptr<Texture2DRegion> GetSubregion(const std::shared_ptr<Texture2DRegion>& textureRegion, const Rectangle& region)
    {
        return GetSubregion(textureRegion, region.X, region.Y, region.Width, region.Height, "");
    }

    std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, const std::string& name, const Rectangle& region)
    {
        return GetSubregion(textureRegion, region.X, region.Y, region.Width, region.Height, name);
    }

    std::shared_ptr<Texture2DRegion> GetSubregion(
        const std::shared_ptr<Texture2DRegion>& textureRegion, int x, int y, int width, int height)
    {
        return GetSubregion(textureRegion, x, y, width, height, "");
    }

    std::shared_ptr<Texture2DRegion> GetSubregion(const std::shared_ptr<Texture2DRegion>& textureRegion, int x, int y,
        int width, int height, const std::string& name)
    {
        if (!textureRegion)
        {
            throw std::invalid_argument("textureRegion must not be null.");
        }

        std::string resolvedName = name;
        if (resolvedName.empty())
        {
            resolvedName = textureRegion->getTextureProperty()->getNameProperty() + "(" + std::to_string(x) + ", " +
                std::to_string(y) + ", " + std::to_string(width) + ", " + std::to_string(height) + ")";
        }

        // The requested subregion is in original sprite coordinates
        const Rectangle requestedRegion(x, y, width, height);

        // Calculate the bounds of the trimmed region in original sprite coordinates
        const Rectangle trimmedBounds(static_cast<int>(textureRegion->getOffsetProperty().X),
            static_cast<int>(textureRegion->getOffsetProperty().Y),
            textureRegion->getIsRotatedProperty() ? textureRegion->getHeightProperty() : textureRegion->getWidthProperty(),
            textureRegion->getIsRotatedProperty() ? textureRegion->getWidthProperty() : textureRegion->getHeightProperty());

        // Find intersection between requested subregion and the actual trimmed region
        const Rectangle intersection = Rectangle::Intersect(requestedRegion, trimmedBounds);

        if (intersection.getIsEmptyProperty())
        {
            return nullptr;
        }

        // The subregion offset must include the existing offset of the input region
        const Vector2 subregionOffset(std::max(0.0f, textureRegion->getOffsetProperty().X - static_cast<float>(x)),
            std::max(0.0f, textureRegion->getOffsetProperty().Y - static_cast<float>(y)));

        if (textureRegion->getIsRotatedProperty())
        {
            // Calculate the actual texture coordinates
            const int textureX = textureRegion->getXProperty() +
                (textureRegion->getWidthProperty() + static_cast<int>(textureRegion->getOffsetProperty().Y) - intersection.getBottomProperty());
            const int textureY = textureRegion->getYProperty() + (intersection.X - static_cast<int>(textureRegion->getOffsetProperty().X));

            return std::make_shared<Texture2DRegion>(textureRegion->getTextureProperty(), textureX, textureY,
                intersection.Height, intersection.Width, textureRegion->getIsRotatedProperty(), Size(width, height),
                subregionOffset, textureRegion->getOriginNormalizedProperty(), resolvedName);
        }
        else
        {
            // Calculate the actual texture coordinates
            const int textureX = textureRegion->getXProperty() + (intersection.X - static_cast<int>(textureRegion->getOffsetProperty().X));
            const int textureY = textureRegion->getYProperty() + (intersection.Y - static_cast<int>(textureRegion->getOffsetProperty().Y));

            return std::make_shared<Texture2DRegion>(textureRegion->getTextureProperty(), textureX, textureY,
                intersection.Width, intersection.Height, textureRegion->getIsRotatedProperty(), Size(width, height),
                subregionOffset, textureRegion->getOriginNormalizedProperty(), resolvedName);
        }
    }

    std::shared_ptr<NinePatch> CreateNinePatch(const std::shared_ptr<Texture2DRegion>& textureRegion, const Thickness& padding)
    {
        return CreateNinePatch(textureRegion, padding.getLeftProperty(), padding.getTopProperty(), padding.getRightProperty(),
            padding.getBottomProperty());
    }

    std::shared_ptr<NinePatch> CreateNinePatch(const std::shared_ptr<Texture2DRegion>& textureRegion, int padding)
    {
        return CreateNinePatch(textureRegion, padding, padding, padding, padding);
    }

    std::shared_ptr<NinePatch> CreateNinePatch(const std::shared_ptr<Texture2DRegion>& textureRegion, int leftPadding,
        int topPadding, int rightPadding, int bottomPadding)
    {
        // Upstream has no explicit ArgumentNullException.ThrowIfNull here -- accessing
        // textureRegion.OriginalSize on a null receiver throws NullReferenceException instead,
        // which the XML doc comment (inaccurately) still calls ArgumentNullException. Checked
        // explicitly here so the failure is a catchable, documented exception rather than a null
        // pointer dereference.
        if (!textureRegion)
        {
            throw std::invalid_argument("textureRegion must not be null.");
        }

        std::vector<std::shared_ptr<Texture2DRegion>> patches(9);

        // Use original sprite dimensions for calculations
        const Size originalSize = textureRegion->getOriginalSizeProperty();
        const int middleWidth = originalSize.Width - leftPadding - rightPadding;
        const int middleHeight = originalSize.Height - topPadding - bottomPadding;
        const int rightX = originalSize.Width - rightPadding;
        const int bottomY = originalSize.Height - bottomPadding;

        patches[NinePatch::TopLeft] = GetSubregion(textureRegion, 0, 0, leftPadding, topPadding);
        patches[NinePatch::TopMiddle] = GetSubregion(textureRegion, leftPadding, 0, middleWidth, topPadding);
        patches[NinePatch::TopRight] = GetSubregion(textureRegion, rightX, 0, rightPadding, topPadding);

        patches[NinePatch::MiddleLeft] = GetSubregion(textureRegion, 0, topPadding, leftPadding, middleHeight);
        patches[NinePatch::Middle] = GetSubregion(textureRegion, leftPadding, topPadding, middleWidth, middleHeight);
        patches[NinePatch::MiddleRight] = GetSubregion(textureRegion, rightX, topPadding, rightPadding, middleHeight);

        patches[NinePatch::BottomLeft] = GetSubregion(textureRegion, 0, bottomY, leftPadding, bottomPadding);
        patches[NinePatch::BottomMiddle] = GetSubregion(textureRegion, leftPadding, bottomY, middleWidth, bottomPadding);
        patches[NinePatch::BottomRight] = GetSubregion(textureRegion, rightX, bottomY, rightPadding, bottomPadding);

        return std::make_shared<NinePatch>(std::move(patches));
    }
}
