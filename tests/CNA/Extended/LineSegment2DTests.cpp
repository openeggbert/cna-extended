// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Upstream's LineSegment2DTest.cs (22 test methods) exercises both self-contained behavior and
// the Collision2D-dependent Distance*/Intersects overloads deferred in LineSegment2D.hpp. These
// tests cover what's actually ported: construction, Direction/Midpoint/Length/LengthSquared,
// GetBounds, GetPoint, ClosestPoint, Deconstruct, Equals/GetHashCode/ToString.
#include "CNA/Extended/LineSegment2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    TEST(LineSegment2DTests, ConstructorSetsStartAndEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.Start, Vector2(0, 0));
        EXPECT_EQ(segment.End, Vector2(10, 0));
    }

    TEST(LineSegment2DTests, DirectionIsEndMinusStart)
    {
        const LineSegment2D segment(Vector2(1, 1), Vector2(4, 5));
        EXPECT_EQ(segment.getDirectionProperty(), Vector2(3, 4));
    }

    TEST(LineSegment2DTests, MidpointIsHalfwayBetweenEndpoints)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.getMidpointProperty(), Vector2(5, 0));
    }

    TEST(LineSegment2DTests, LengthMatchesDistance)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        EXPECT_FLOAT_EQ(segment.getLengthProperty(), 5.0f);
    }

    TEST(LineSegment2DTests, LengthSquaredAvoidsSquareRoot)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        EXPECT_FLOAT_EQ(segment.getLengthSquaredProperty(), 25.0f);
    }

    TEST(LineSegment2DTests, GetBoundsEnclosesEndpoints)
    {
        const LineSegment2D segment(Vector2(5, -2), Vector2(-1, 8));
        const BoundingBox2D bounds = segment.GetBounds();
        EXPECT_EQ(bounds.Min, Vector2(-1, -2));
        EXPECT_EQ(bounds.Max, Vector2(5, 8));
    }

    TEST(LineSegment2DTests, GetPointInterpolatesAlongSegment)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(segment.GetPoint(0.0f), Vector2(0, 0));
        EXPECT_EQ(segment.GetPoint(0.5f), Vector2(5, 0));
        EXPECT_EQ(segment.GetPoint(1.0f), Vector2(10, 0));
    }

    TEST(LineSegment2DTests, ClosestPointClampsToStart)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(-5, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(0, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 0.0f);
    }

    TEST(LineSegment2DTests, ClosestPointClampsToEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(15, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(10, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 1.0f);
    }

    TEST(LineSegment2DTests, ClosestPointProjectsOntoSegment)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        float distanceAlongSegment = -1.0f;
        const Vector2 closest = segment.ClosestPoint(Vector2(5, 3), distanceAlongSegment);
        EXPECT_EQ(closest, Vector2(5, 0));
        EXPECT_FLOAT_EQ(distanceAlongSegment, 0.5f);
    }

    TEST(LineSegment2DTests, DeconstructReturnsStartAndEnd)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(10, 0));
        Vector2 start, end;
        segment.Deconstruct(start, end);
        EXPECT_EQ(start, Vector2(0, 0));
        EXPECT_EQ(end, Vector2(10, 0));
    }

    TEST(LineSegment2DTests, EqualsAndOperators)
    {
        const LineSegment2D a(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D b(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D c(Vector2(0, 0), Vector2(5, 0));

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(LineSegment2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const LineSegment2D a(Vector2(0, 0), Vector2(10, 0));
        const LineSegment2D b(Vector2(0, 0), Vector2(10, 0));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(LineSegment2DTests, ToStringContainsStartEndAndLength)
    {
        const LineSegment2D segment(Vector2(0, 0), Vector2(3, 4));
        const std::string text = segment.ToString();
        EXPECT_NE(text.find("Start"), std::string::npos);
        EXPECT_NE(text.find("End"), std::string::npos);
        EXPECT_NE(text.find("Length"), std::string::npos);
    }
}
