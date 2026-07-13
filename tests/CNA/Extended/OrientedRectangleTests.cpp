// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's tests/MonoGame.Extended.Tests/Primitives/
// OrientedRectangleTests.cs. The nested `Transform` test class (11 tests) is entirely about
// OrientedRectangle::Transform, which is deferred (see OrientedRectangle.hpp) -- not ported.
// The two portable upstream tests (initialization, equality) are ported 1:1 below; the
// self-contained Intersects(...) SAT test has no upstream coverage, so fresh tests were written
// for it.
#include "CNA/Extended/OrientedRectangle.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(OrientedRectangleTests, InitializesOrientedRectangle)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_EQ(rectangle.Center, Vector2(1, 2));
        EXPECT_EQ(rectangle.Radii, Vector2(3, 4));
        EXPECT_EQ(rectangle.Orientation, Matrix3x2(5, 6, 7, 8, 9, 10));

        const std::vector<Vector2> expectedPoints = {
            Vector2(-3, -2),
            Vector2(-33, -38),
            Vector2(23, 26),
            Vector2(53, 62),
        };
        EXPECT_EQ(rectangle.getPointsProperty(), expectedPoints);
    }

    TEST(OrientedRectangleTests, EqualsComparisonEmptyComparedWithEmptyIsTrue)
    {
        const OrientedRectangle first(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);
        const OrientedRectangle second(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);

        EXPECT_TRUE(first == second);
        EXPECT_FALSE(first != second);
    }

    TEST(OrientedRectangleTests, EqualsComparisonInitializedComparedWithInitializedIsTrue)
    {
        const OrientedRectangle first(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));
        const OrientedRectangle second(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_TRUE(first == second);
        EXPECT_FALSE(first != second);
    }

    TEST(OrientedRectangleTests, GetHashCodeMatchesForEqualInstances)
    {
        const OrientedRectangle first(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));
        const OrientedRectangle second(Vector2(1, 2), SizeF(3, 4), Matrix3x2(5, 6, 7, 8, 9, 10));

        EXPECT_EQ(first.GetHashCode(), second.GetHashCode());
    }

    TEST(OrientedRectangleTests, PositionGetterMatchesUpstreamFormula)
    {
        const OrientedRectangle rectangle(Vector2(1, 2), SizeF(3, 4), Matrix3x2::Identity);
        EXPECT_EQ(rectangle.getPositionProperty(), Vector2::Transform(-rectangle.Radii, rectangle.Orientation) + rectangle.Center);
    }

    TEST(OrientedRectangleTests, PositionSetterThrows)
    {
        OrientedRectangle rectangle(Vector2::Zero, SizeF::Empty, Matrix3x2::Identity);
        EXPECT_THROW(rectangle.setPositionProperty(Vector2::Zero), std::logic_error);
    }

    TEST(OrientedRectangleTests, ConvertsFromAxisAlignedRectangleF)
    {
        const RectangleF source(0.0f, 0.0f, 10.0f, 20.0f);
        const OrientedRectangle rectangle(source);

        EXPECT_EQ(rectangle.Center, Vector2(5.0f, 10.0f));
        EXPECT_EQ(rectangle.Radii, Vector2(5.0f, 10.0f));
        EXPECT_EQ(rectangle.Orientation, Matrix3x2::Identity);
    }

    TEST(OrientedRectangleTests, IntersectsDetectsOverlappingRectangles)
    {
        const OrientedRectangle a(Vector2::Zero, SizeF(5, 5), Matrix3x2::Identity);
        const OrientedRectangle b(Vector2(4, 0), SizeF(5, 5), Matrix3x2::Identity);

        const OrientedRectangleIntersection result = OrientedRectangle::Intersects(a, b);
        EXPECT_TRUE(result.Intersects);
    }

    TEST(OrientedRectangleTests, IntersectsDetectsNonOverlappingRectangles)
    {
        const OrientedRectangle a(Vector2::Zero, SizeF(5, 5), Matrix3x2::Identity);
        const OrientedRectangle b(Vector2(100, 0), SizeF(5, 5), Matrix3x2::Identity);

        const OrientedRectangleIntersection result = OrientedRectangle::Intersects(a, b);
        EXPECT_FALSE(result.Intersects);
    }
}
