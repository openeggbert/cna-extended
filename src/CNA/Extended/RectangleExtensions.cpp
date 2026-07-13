// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/RectangleExtensions.hpp"

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <algorithm>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::MathHelper;

    std::array<Point, 4> GetCorners(const Rectangle& rectangle)
    {
        std::array<Point, 4> corners{};
        corners[0] = Point(rectangle.getLeftProperty(), rectangle.getTopProperty());
        corners[1] = Point(rectangle.getRightProperty(), rectangle.getTopProperty());
        corners[2] = Point(rectangle.getRightProperty(), rectangle.getBottomProperty());
        corners[3] = Point(rectangle.getLeftProperty(), rectangle.getBottomProperty());
        return corners;
    }

    RectangleF ToRectangleF(const Rectangle& rectangle)
    {
        return RectangleF(static_cast<float>(rectangle.X), static_cast<float>(rectangle.Y),
            static_cast<float>(rectangle.Width), static_cast<float>(rectangle.Height));
    }

    Rectangle Clip(const Rectangle& rectangle, const Rectangle& clippingRectangle)
    {
        const int left = std::max(rectangle.getLeftProperty(), clippingRectangle.getLeftProperty());
        const int top = std::max(rectangle.getTopProperty(), clippingRectangle.getTopProperty());
        const int right = std::min(rectangle.getRightProperty(), clippingRectangle.getRightProperty());
        const int bottom = std::min(rectangle.getBottomProperty(), clippingRectangle.getBottomProperty());

        const int width = right - left;
        const int height = bottom - top;

        if (width <= 0 || height <= 0)
        {
            return Rectangle::Empty;
        }

        return Rectangle(left, top, width, height);
    }

    Rectangle GetRelativeRectangle(const Rectangle& source, const int x, const int y, const int width, const int height)
    {
        const int absoluteX = source.X + x;
        const int absoluteY = source.Y + y;

        Rectangle relative;
        relative.X = static_cast<int>(MathHelper::Clamp(static_cast<float>(absoluteX), static_cast<float>(source.getLeftProperty()),
            static_cast<float>(source.getRightProperty())));
        relative.Y = static_cast<int>(MathHelper::Clamp(static_cast<float>(absoluteY), static_cast<float>(source.getTopProperty()),
            static_cast<float>(source.getBottomProperty())));
        relative.Width = std::max(std::min(absoluteX + width, source.getRightProperty()) - relative.X, 0);
        relative.Height = std::max(std::min(absoluteY + height, source.getBottomProperty()) - relative.Y, 0);

        return relative;
    }

    void Deconstruct(const Rectangle& rectangle, int& x, int& y, int& width, int& height)
    {
        x = rectangle.X;
        y = rectangle.Y;
        width = rectangle.Width;
        height = rectangle.Height;
    }

    Rectangle Normalize(Rectangle rectangle)
    {
        if (rectangle.Width < 0)
        {
            rectangle.X += rectangle.Width;
            rectangle.Width = -rectangle.Width;
        }

        if (rectangle.Height < 0)
        {
            rectangle.Y += rectangle.Height;
            rectangle.Height = -rectangle.Height;
        }

        return rectangle;
    }

    void Normalize(const Rectangle& rectangle, Rectangle& result)
    {
        result.X = rectangle.X;
        result.Width = rectangle.Width;

        if (result.Width < 0)
        {
            result.X += result.Width;
            result.Width = -result.Width;
        }

        result.Y = rectangle.Y;
        result.Height = rectangle.Height;

        if (result.Height < 0)
        {
            result.Y += result.Height;
            result.Height = -result.Height;
        }
    }
}
