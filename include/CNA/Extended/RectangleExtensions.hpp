// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Rectangle.Extensions.cs. Extension methods -> free functions
// in this namespace, matching the convention used throughout this project (see ColorExtensions.hpp).
// Upstream's `#if FNA` Deconstruct compatibility method is ported unconditionally: CNA mirrors
// FNA's API surface (see CLAUDE.md), so the FNA branch is this project's baseline, not an
// optional variant.
#pragma once

#include "Microsoft/Xna/Framework/Point.hpp"
#include "Microsoft/Xna/Framework/Rectangle.hpp"

#include <array>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Point;
    using Microsoft::Xna::Framework::Rectangle;

    /** @brief Gets the corners of the rectangle in a clockwise direction starting at the top left. */
    [[nodiscard]] std::array<Point, 4> GetCorners(const Rectangle& rectangle);

    /** @brief Converts the specified Rectangle to a RectangleF. */
    class RectangleF;
    [[nodiscard]] RectangleF ToRectangleF(const Rectangle& rectangle);

    /**
     * @brief Clips the specified rectangle against the specified clipping rectangle.
     * @return The clipped rectangle, or Rectangle::Empty if the rectangles do not intersect.
     */
    [[nodiscard]] Rectangle Clip(const Rectangle& rectangle, const Rectangle& clippingRectangle);

    /**
     * @brief Gets a rectangle that is relative to the specified source rectangle, with the
     * specified offsets and dimensions, clipped to the source rectangle.
     */
    [[nodiscard]] Rectangle GetRelativeRectangle(const Rectangle& source, int x, int y, int width, int height);

    /** @brief Deconstructs this rectangle into its component values (FNA compatibility layer). */
    void Deconstruct(const Rectangle& rectangle, int& x, int& y, int& width, int& height);

    /**
     * @brief Normalizes the specified Rectangle so that Width and Height are positive without
     * changing the location of the rectangle.
     */
    [[nodiscard]] Rectangle Normalize(Rectangle rectangle);

    /**
     * @brief Normalizes a Rectangle so that Width and Height are positive without changing the
     * location of the rectangle.
     * @param result Receives the normalized rectangle.
     */
    void Normalize(const Rectangle& rectangle, Rectangle& result);
}
