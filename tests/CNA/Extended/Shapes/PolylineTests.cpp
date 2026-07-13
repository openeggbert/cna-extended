// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for Polyline.cs. These tests cover
// bounds/BoundingRectangle and the always-false Contains stub.
#include "CNA/Extended/Shapes/Polyline.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Shapes
{
    TEST(PolylineTests, BoundsReflectPoints)
    {
        const Polyline polyline({Vector2(1, 5), Vector2(4, 2), Vector2(7, 8)});
        EXPECT_FLOAT_EQ(polyline.getLeftProperty(), 1.0f);
        EXPECT_FLOAT_EQ(polyline.getTopProperty(), 2.0f);
        EXPECT_FLOAT_EQ(polyline.getRightProperty(), 7.0f);
        EXPECT_FLOAT_EQ(polyline.getBottomProperty(), 8.0f);
    }

    TEST(PolylineTests, BoundingRectangleEnclosesPoints)
    {
        const Polyline polyline({Vector2(1, 5), Vector2(4, 2), Vector2(7, 8)});
        const RectangleF bounds = polyline.getBoundingRectangleProperty();
        EXPECT_FLOAT_EQ(bounds.X, 1.0f);
        EXPECT_FLOAT_EQ(bounds.Y, 2.0f);
        EXPECT_FLOAT_EQ(bounds.Width, 6.0f);
        EXPECT_FLOAT_EQ(bounds.Height, 6.0f);
    }

    TEST(PolylineTests, ContainsAlwaysReturnsFalse)
    {
        // Matches upstream: Polyline.Contains is a stub that always returns false (an open curve
        // has no interior); not "fixed" into a real geometric test.
        const Polyline polyline({Vector2(0, 0), Vector2(10, 10)});
        EXPECT_FALSE(polyline.Contains(5.0f, 5.0f));
        EXPECT_FALSE(polyline.Contains(Vector2(5, 5)));
    }
}
