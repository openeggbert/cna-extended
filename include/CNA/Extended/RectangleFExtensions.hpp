// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's RectangleF.Extensions.cs. Extension methods -> free functions,
// matching the convention used throughout this project.
//
// *** LIKELY UPSTREAM BUG, PRESERVED FOR FIDELITY -- flagged prominently, not silently fixed ***
// Clip(rectangle, clippingRectangle) does NOT compute a true geometric intersection (unlike
// RectangleExtensions::Clip, the analogous method for the integer Rectangle type, which uses
// proper min/max intersection logic). Instead it mutates X/Y first, then derives Width/Height
// from the Right/Bottom properties computed off the *already-mutated* X/Y -- not the original
// rectangle's bounds. Concretely verified (see Clip's implementation comment and
// RectangleFExtensionsTests.cpp's regression tests for hand-traced examples): when only the
// left/top edge needs clipping and the right/bottom edge does not, Width/Height are left
// unchanged instead of shrinking to account for the moved left/top edge, producing a rectangle
// larger than the true intersection. When the clip rectangle doesn't overlap the source at all,
// Clip does not detect this and does not return RectangleF::Empty -- it silently produces a
// bogus non-empty "intersection" positioned at the clip rectangle's own X/Y. This is upstream's
// actual behavior, reproduced exactly below rather than "fixed" to be geometrically correct.
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
     * @warning NOT a true geometric intersection -- see this file's header comment for a known
     * upstream fidelity bug this method reproduces exactly (mutates X/Y before deriving
     * Width/Height from them, and does not detect true non-overlap).
     */
    [[nodiscard]] RectangleF Clip(RectangleF rectangle, const RectangleF& clippingRectangle);

    /**
     * @brief Gets a rectangle that is relative to the specified source rectangle, with the
     * specified offsets and dimensions, clipped to the source rectangle.
     */
    [[nodiscard]] RectangleF GetRelativeRectangle(const RectangleF& source, float x, float y, float width, float height);
}
