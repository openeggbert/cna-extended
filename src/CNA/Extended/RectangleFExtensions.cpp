// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/RectangleFExtensions.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <algorithm>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    std::array<Vector2, 4> GetCorners(const RectangleF& rectangle)
    {
        std::array<Vector2, 4> corners{};
        corners[0] = Vector2(rectangle.getLeftProperty(), rectangle.getTopProperty());
        corners[1] = Vector2(rectangle.getRightProperty(), rectangle.getTopProperty());
        corners[2] = Vector2(rectangle.getRightProperty(), rectangle.getBottomProperty());
        corners[3] = Vector2(rectangle.getLeftProperty(), rectangle.getBottomProperty());
        return corners;
    }

    Rectangle ToRectangle(const RectangleF& rectangle)
    {
        return Rectangle(static_cast<int>(rectangle.X), static_cast<int>(rectangle.Y), static_cast<int>(rectangle.Width),
            static_cast<int>(rectangle.Height));
    }

    RectangleF Clip(RectangleF rectangle, const RectangleF& clippingRectangle)
    {
        const RectangleF& clip = clippingRectangle;
        rectangle.X = clip.X > rectangle.X ? clip.X : rectangle.X;
        rectangle.Y = clip.Y > rectangle.Y ? clip.Y : rectangle.Y;
        rectangle.Width = rectangle.getRightProperty() > clip.getRightProperty() ? clip.getRightProperty() - rectangle.X : rectangle.Width;
        rectangle.Height =
            rectangle.getBottomProperty() > clip.getBottomProperty() ? clip.getBottomProperty() - rectangle.Y : rectangle.Height;

        if (rectangle.Width <= 0 || rectangle.Height <= 0)
        {
            return RectangleF::Empty;
        }

        return rectangle;
    }

    RectangleF GetRelativeRectangle(const RectangleF& source, const float x, const float y, const float width, const float height)
    {
        const float absoluteX = source.X + x;
        const float absoluteY = source.Y + y;

        RectangleF relative;
        relative.X = MathHelper::Clamp(absoluteX, source.getLeftProperty(), source.getRightProperty());
        relative.Y = MathHelper::Clamp(absoluteY, source.getTopProperty(), source.getBottomProperty());
        relative.Width = std::max(std::min(absoluteX + width, source.getRightProperty()) - relative.X, 0.0f);
        relative.Height = std::max(std::min(absoluteY + height, source.getBottomProperty()) - relative.Y, 0.0f);

        return relative;
    }
}
