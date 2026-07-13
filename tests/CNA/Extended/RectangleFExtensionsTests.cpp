// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream test file for RectangleF.Extensions.cs. These
// tests cover GetCorners/ToRectangle/Clip/GetRelativeRectangle, mirroring the coverage/style
// upstream's RectangleExtensionsTests.cs gives the analogous Rectangle methods.
#include "CNA/Extended/RectangleFExtensions.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(RectangleFExtensionsTests, GetCornersReturnsClockwiseFromTopLeft)
    {
        const RectangleF rectangle(0, 0, 10, 20);
        const auto corners = GetCorners(rectangle);

        EXPECT_EQ(corners[0], Vector2(0, 0));
        EXPECT_EQ(corners[1], Vector2(10, 0));
        EXPECT_EQ(corners[2], Vector2(10, 20));
        EXPECT_EQ(corners[3], Vector2(0, 20));
    }

    TEST(RectangleFExtensionsTests, ToRectangleTruncatesToInt)
    {
        const RectangleF rectangle(1.9f, 2.9f, 10.9f, 20.9f);
        const Rectangle actual = ToRectangle(rectangle);

        EXPECT_EQ(actual, Rectangle(1, 2, 10, 20));
    }

    // Unlike RectangleExtensions::Clip (which uses proper min/max intersection), upstream's
    // RectangleF.Extensions.Clip mutates X/Y first and then computes Width/Height from the
    // *already-updated* X/Y (via the Right/Bottom properties) -- a different, less robust
    // algorithm. These expected values were hand-traced through that exact sequential mutation,
    // not computed as a "true" geometric intersection.
    TEST(RectangleFExtensionsTests, ClipReturnsIntersectionRectangle)
    {
        const RectangleF rect(0, 0, 10, 10);
        const RectangleF clip1(2, 2, 5, 5);
        const RectangleF clip2(2, 2, 15, 15);

        EXPECT_EQ(Clip(rect, clip1), RectangleF(2, 2, 5, 5));
        // X,Y become (2,2) first; Width/Height are then re-checked against clip's Right/Bottom
        // using the *new* X/Y (rect.Right becomes 2+10=12, clip2.Right=17; 12 is not > 17, so
        // Width is left unchanged at 10) -- so the result is (2,2,10,10), not a (2,2,8,8)
        // "true" intersection.
        EXPECT_EQ(Clip(rect, clip2), RectangleF(2, 2, 10, 10));
    }

    // Demonstrates the same algorithm's limitation: when the clip rectangle doesn't overlap the
    // source at all, upstream's Clip does NOT detect this and does NOT return RectangleF::Empty
    // -- it unconditionally moves X/Y to the clip rectangle's position, and since rect.Right/
    // Bottom (recomputed from the new X/Y) don't exceed clip.Right/Bottom, Width/Height are left
    // unchanged, producing a bogus non-empty "intersection". This is upstream's actual behavior,
    // preserved faithfully rather than "fixed" to be geometrically correct.
    TEST(RectangleFExtensionsTests, ClipWithNoRealOverlapDoesNotReturnEmpty)
    {
        const RectangleF rect(0, 0, 10, 10);
        const RectangleF clip(100, 100, 5, 5);

        EXPECT_EQ(Clip(rect, clip), RectangleF(100, 100, 5, 5));
    }

    TEST(RectangleFExtensionsTests, GetRelativeRectangleClipsToSource)
    {
        const RectangleF source(0, 0, 100, 100);
        const RectangleF relative = GetRelativeRectangle(source, 90, 90, 20, 20);

        EXPECT_FLOAT_EQ(relative.X, 90.0f);
        EXPECT_FLOAT_EQ(relative.Y, 90.0f);
        EXPECT_FLOAT_EQ(relative.Width, 10.0f);
        EXPECT_FLOAT_EQ(relative.Height, 10.0f);
    }
}
