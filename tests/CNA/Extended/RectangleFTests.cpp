// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Math/RectangleFTests.cs
// (Normalize + the GitHub issue #747 regression test). The separate upstream
// tests/MonoGame.Extended.Tests/Primitives/RectangleFTests.cs is NOT ported here: every test in
// it constructs a RectangleF from a SizeF or exercises RectangleF::Transform(Matrix3x2), both
// deferred (see RectangleF.hpp's header comment) until SizeF/Matrix3x2 land.
#include "CNA/Extended/RectangleF.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(RectangleFTests, NormalizeWithNegativeWidthAndHeightAdjustsPositionAndMakesDimensionsPositive)
    {
        RectangleF rectangle(32, 32, -32, -32);
        rectangle.Normalize();

        const RectangleF expected(0, 0, 32, 32);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithNegativeWidthOnlyAdjustsXAndMakesWidthPositive)
    {
        RectangleF rectangle(100, 50, -20, 30);
        rectangle.Normalize();

        const RectangleF expected(80, 50, 20, 30);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithNegativeHeightOnlyAdjustsYAndMakesHeightPositive)
    {
        RectangleF rectangle(50, 100, 30, -20);
        rectangle.Normalize();

        const RectangleF expected(50, 80, 30, 20);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeWithPositiveDimensionsDoesNotModifyRectangle)
    {
        RectangleF rectangle(10, 20, 30, 40);
        rectangle.Normalize();

        const RectangleF expected(10, 20, 30, 40);
        EXPECT_EQ(expected, rectangle);
    }

    TEST(RectangleFTests, NormalizeStaticMethodReturnsNormalizedRectangle)
    {
        const RectangleF rectangle(32, 32, -32, -32);

        const RectangleF actual = RectangleF::Normalize(rectangle);
        const RectangleF expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }

    TEST(RectangleFTests, NormalizeRefOutMethodOutputsNormalizedRectangle)
    {
        const RectangleF rectangle(32, 32, -32, -32);

        RectangleF actual;
        RectangleF::Normalize(rectangle, actual);
        const RectangleF expected(0, 0, 32, 32);

        EXPECT_EQ(expected, actual);
    }

    // GitHub issue #747/#793 regression test.
    TEST(RectangleFIssue747Tests, NormalizeFixesIntersectionIssueFromGitHubIssue747)
    {
        RectangleF shape1(32, 32, -32, -32);
        RectangleF shape2(16, 16, -32, -32);

        shape1.Normalize();
        shape2.Normalize();

        EXPECT_TRUE(shape1.Intersects(shape2));
    }
}
