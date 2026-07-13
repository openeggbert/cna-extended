// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended's tests/MonoGame.Extended.Tests/Primitives/BoundingRectangleTests.cs is
// entirely commented out upstream (a disabled NUnit-style test file, not currently run by
// upstream's own test suite) -- there is nothing active to port 1:1. These tests cover what was
// actually ported (CreateFrom, Union, Intersection, Intersects, Contains,
// Equals/GetHashCode/ToString, the Rectangle/RectangleF conversions), written fresh but informed
// by the disabled upstream cases' intent where still applicable. CreateFrom(points), Transform,
// UpdateFromPoints, SquaredDistanceTo, and ClosestPointTo were added once Matrix3x2 and
// PrimitivesHelper landed (task 22).
#include "CNA/Extended/BoundingRectangle.hpp"

#include "CNA/Extended/Matrix3x2.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(BoundingRectangleTests, CreateFromMinimumMaximum)
    {
        const BoundingRectangle result = BoundingRectangle::CreateFrom(Vector2(5, 5), Vector2(15, 15));
        EXPECT_EQ(result.Center, Vector2(10, 10));
        EXPECT_EQ(result.HalfExtents, Vector2(5, 5));
    }

    TEST(BoundingRectangleTests, EmptyCreatedFromZeroMinimumMaximum)
    {
        const BoundingRectangle result = BoundingRectangle::CreateFrom(Vector2::Zero, Vector2::Zero);
        EXPECT_EQ(result, BoundingRectangle::Empty);
    }

    TEST(BoundingRectangleTests, UnionOfTwoNonEmptyRectanglesEnclosesBoth)
    {
        const BoundingRectangle a = BoundingRectangle::CreateFrom(Vector2(0, 0), Vector2(15, 15));
        const BoundingRectangle b = BoundingRectangle::CreateFrom(Vector2(20, 20), Vector2(40, 40));

        const BoundingRectangle expected = BoundingRectangle::CreateFrom(Vector2(0, 0), Vector2(40, 40));
        EXPECT_EQ(a.Union(b), expected);
        EXPECT_EQ(BoundingRectangle::Union(a, b), expected);
    }

    TEST(BoundingRectangleTests, IntersectionOfOverlappingRectangles)
    {
        const BoundingRectangle a = BoundingRectangle::CreateFrom(Vector2(-10, -10), Vector2(5, 5));
        const BoundingRectangle b = BoundingRectangle::CreateFrom(Vector2(0, 0), Vector2(20, 20));

        const BoundingRectangle expected = BoundingRectangle::CreateFrom(Vector2(0, 0), Vector2(5, 5));
        EXPECT_EQ(a.Intersection(b), expected);
        EXPECT_EQ(BoundingRectangle::Intersection(a, b), expected);
    }

    TEST(BoundingRectangleTests, IntersectionOfNonOverlappingRectanglesIsEmpty)
    {
        const BoundingRectangle a = BoundingRectangle::CreateFrom(Vector2(-30, -30), Vector2(-15, -15));
        const BoundingRectangle b = BoundingRectangle::CreateFrom(Vector2(20, 20), Vector2(30, 30));

        EXPECT_EQ(a.Intersection(b), BoundingRectangle::Empty);
    }

    TEST(BoundingRectangleTests, IntersectsOverlapping)
    {
        const BoundingRectangle a = BoundingRectangle::CreateFrom(Vector2(-10, -10), Vector2(5, 5));
        const BoundingRectangle b = BoundingRectangle::CreateFrom(Vector2(0, 0), Vector2(40, 40));

        EXPECT_TRUE(a.Intersects(b));
        EXPECT_TRUE(BoundingRectangle::Intersects(a, b));
    }

    TEST(BoundingRectangleTests, IntersectsNonOverlapping)
    {
        const BoundingRectangle a = BoundingRectangle::CreateFrom(Vector2(-40, -50), Vector2(-25, -35));
        const BoundingRectangle b = BoundingRectangle::CreateFrom(Vector2(20, 20), Vector2(35, 35));

        EXPECT_FALSE(a.Intersects(b));
    }

    TEST(BoundingRectangleTests, ContainsPointInside)
    {
        const BoundingRectangle rectangle = BoundingRectangle::CreateFrom(Vector2(-15, -15), Vector2(15, 15));
        EXPECT_TRUE(rectangle.Contains(Vector2(-15, -15)));
        EXPECT_TRUE(BoundingRectangle::Contains(rectangle, Vector2(0, 0)));
    }

    TEST(BoundingRectangleTests, ContainsPointOutside)
    {
        const BoundingRectangle rectangle = BoundingRectangle::CreateFrom(Vector2(-15, -15), Vector2(15, 15));
        EXPECT_FALSE(rectangle.Contains(Vector2(-16, 15)));
    }

    TEST(BoundingRectangleTests, EqualsAndOperators)
    {
        const BoundingRectangle a(Vector2(0, 0), Vector2(15, 15));
        const BoundingRectangle b(Vector2(0, 0), Vector2(15, 15));
        const BoundingRectangle c(Vector2(1, 0), Vector2(15, 15));

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
        EXPECT_TRUE(a.Equals(b));
    }

    TEST(BoundingRectangleTests, GetHashCodeMatchesForEqualInstances)
    {
        const BoundingRectangle a(Vector2(0, 0), Vector2(50, 50));
        const BoundingRectangle b(Vector2(0, 0), Vector2(50, 50));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(BoundingRectangleTests, ToRectangleConversion)
    {
        const BoundingRectangle rectangle(Vector2(25, 25), Vector2(25, 25));
        const auto converted = static_cast<Rectangle>(rectangle);
        EXPECT_EQ(converted, Rectangle(0, 0, 50, 50));
    }

    TEST(BoundingRectangleTests, ToRectangleFConversion)
    {
        const BoundingRectangle rectangle(Vector2(25, 25), Vector2(25, 25));
        const auto converted = static_cast<RectangleF>(rectangle);
        EXPECT_EQ(converted, RectangleF(0, 0, 50, 50));
    }

    TEST(BoundingRectangleTests, ToStringIncludesCentreAndRadii)
    {
        const BoundingRectangle rectangle(Vector2(5.1f, -5.123f), Vector2(5.4f, -5.4123f));
        const std::string text = rectangle.ToString();
        EXPECT_NE(text.find("Centre"), std::string::npos);
        EXPECT_NE(text.find("Radii"), std::string::npos);
    }

    TEST(BoundingRectangleTests, CreateFromPointsComputesBoundingRectangle)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(1, 8), Vector2(6, 2)};
        const BoundingRectangle result = BoundingRectangle::CreateFrom(points);

        const BoundingRectangle expected = BoundingRectangle::CreateFrom(Vector2(1, 2), Vector2(6, 8));
        EXPECT_EQ(expected, result);
    }

    TEST(BoundingRectangleTests, UpdateFromPointsMutatesRectangle)
    {
        const std::vector<Vector2> points = {Vector2(3, 4), Vector2(1, 8), Vector2(6, 2)};
        BoundingRectangle rectangle;
        rectangle.UpdateFromPoints(points);

        const BoundingRectangle expected = BoundingRectangle::CreateFrom(Vector2(1, 2), Vector2(6, 8));
        EXPECT_EQ(expected, rectangle);
    }

    TEST(BoundingRectangleTests, TransformTranslatesCenter)
    {
        BoundingRectangle rectangle(Vector2(0, 0), Vector2(10, 15));
        Matrix3x2 transform = Matrix3x2::CreateTranslation(1, 2);

        const BoundingRectangle result = BoundingRectangle::Transform(rectangle, transform);

        EXPECT_EQ(result.Center, Vector2(1, 2));
        EXPECT_EQ(result.HalfExtents, Vector2(10, 15));
    }

    TEST(BoundingRectangleTests, SquaredDistanceToIsZeroWhenPointInside)
    {
        const BoundingRectangle rectangle(Vector2(0, 0), Vector2(5, 5));
        EXPECT_FLOAT_EQ(rectangle.SquaredDistanceTo(Vector2(0, 0)), 0.0f);
    }

    TEST(BoundingRectangleTests, ClosestPointToClampsToBoundary)
    {
        const BoundingRectangle rectangle(Vector2(0, 0), Vector2(5, 5));
        EXPECT_EQ(rectangle.ClosestPointTo(Vector2(20, 0)), Vector2(5, 0));
    }
}
