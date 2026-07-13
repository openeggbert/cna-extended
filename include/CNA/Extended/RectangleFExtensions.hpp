// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's RectangleF.Extensions.cs. Extension methods -> free functions,
// matching the convention used throughout this project.
#pragma once

#include "CNA/Extended/RectangleF.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <array>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Rectangle;
    using Microsoft::Xna::Framework::Vector2;

    /** @brief Gets the corners of the rectangle in a clockwise direction starting at the top left. */
    [[nodiscard]] std::array<Vector2, 4> GetCorners(const RectangleF& rectangle);

    /** @brief Converts the specified RectangleF to a Rectangle. */
    [[nodiscard]] Rectangle ToRectangle(const RectangleF& rectangle);

    /**
     * @brief Clips the specified rectangle against the specified clipping rectangle.
     * @return The clipped rectangle, or RectangleF::Empty if the rectangles do not intersect.
     */
    [[nodiscard]] RectangleF Clip(RectangleF rectangle, const RectangleF& clippingRectangle);

    /**
     * @brief Gets a rectangle that is relative to the specified source rectangle, with the
     * specified offsets and dimensions, clipped to the source rectangle.
     */
    [[nodiscard]] RectangleF GetRelativeRectangle(const RectangleF& source, float x, float y, float width, float height);
}
