// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for LineSegment.cs. These tests cover
// FindIntersection and IntersectsWithRay directly.
#include "CNA/Extended/Triangulation/LineSegment.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    TEST(LineSegmentTests, FindIntersectionReturnsCrossingPoint)
    {
        const LineSegment a(Vertex(Vector2(0, 0), 0), Vertex(Vector2(10, 10), 1));
        const LineSegment b(Vertex(Vector2(0, 10), 0), Vertex(Vector2(10, 0), 1));

        const std::optional<Vector2> intersection = LineSegment::FindIntersection(a, b);
        ASSERT_TRUE(intersection.has_value());
        EXPECT_EQ(intersection.value(), Vector2(5, 5));
    }

    TEST(LineSegmentTests, FindIntersectionReturnsNulloptWhenOutsideBothSegments)
    {
        const LineSegment a(Vertex(Vector2(0, 0), 0), Vertex(Vector2(1, 0), 1));
        const LineSegment b(Vertex(Vector2(2, -1), 0), Vertex(Vector2(2, 1), 1));

        EXPECT_FALSE(LineSegment::FindIntersection(a, b).has_value());
    }

    TEST(LineSegmentTests, IntersectsWithRayReturnsDistanceWhenHit)
    {
        const LineSegment segment(Vertex(Vector2(10, 0), 0), Vertex(Vector2(10, 10), 1));

        const std::optional<float> distance = segment.IntersectsWithRay(Vector2(0, 5), Vector2(1, 0));
        ASSERT_TRUE(distance.has_value());
        EXPECT_FLOAT_EQ(distance.value(), 10.0f);
    }

    TEST(LineSegmentTests, IntersectsWithRayReturnsNulloptWhenRayPointsAway)
    {
        const LineSegment segment(Vertex(Vector2(10, 0), 0), Vertex(Vector2(10, 10), 1));

        EXPECT_FALSE(segment.IntersectsWithRay(Vector2(0, 5), Vector2(-1, 0)).has_value());
    }
}
