// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended's own tests/MonoGame.Extended.Tests/Primitives/Segment2DTests.cs is entirely
// commented out upstream itself -- nothing active to port 1:1. All tests below are fresh,
// covering what's actually ported (ClosestPointTo/SquaredDistanceTo/DistanceTo/Equals/
// GetHashCode/ToString/operators). Intersects(RectangleF|BoundingRectangle, out) are deferred
// (see Segment2.hpp) so not tested. One test specifically demonstrates the known-upstream-bug
// behavior in SquaredDistanceTo documented in Segment2.hpp's header comment.
#include "CNA/Extended/Segment2.hpp"

#include <gtest/gtest.h>

#include <cmath>

namespace CNA::Extended
{
    TEST(Segment2Tests, ConstructorFromPoints)
    {
        const Segment2 segment(Vector2(1, 2), Vector2(3, 4));
        EXPECT_EQ(segment.Start, Vector2(1, 2));
        EXPECT_EQ(segment.End, Vector2(3, 4));
    }

    TEST(Segment2Tests, ConstructorFromCoordinates)
    {
        const Segment2 segment(1.0f, 2.0f, 3.0f, 4.0f);
        EXPECT_EQ(segment.Start, Vector2(1, 2));
        EXPECT_EQ(segment.End, Vector2(3, 4));
    }

    TEST(Segment2Tests, ClosestPointBeyondStartIsStart)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(200, 0));
        EXPECT_EQ(segment.ClosestPointTo(Vector2(-100, 200)), Vector2(0, 0));
    }

    TEST(Segment2Tests, ClosestPointBeyondEndIsEnd)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(200, 0));
        EXPECT_EQ(segment.ClosestPointTo(Vector2(400, 200)), Vector2(200, 0));
    }

    TEST(Segment2Tests, ClosestPointInsideIsProjectedPoint)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(200, 0));
        EXPECT_EQ(segment.ClosestPointTo(Vector2(100, 200)), Vector2(100, 0));
    }

    TEST(Segment2Tests, SquaredDistanceBeyondStartIsDistanceToStart)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(20, 0));
        EXPECT_FLOAT_EQ(segment.SquaredDistanceTo(Vector2(-10, 20)), 500.0f);
    }

    TEST(Segment2Tests, SquaredDistanceInsideIsDistanceToProjectedPoint)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(20, 0));
        EXPECT_FLOAT_EQ(segment.SquaredDistanceTo(Vector2(10, 25)), 625.0f);
    }

    TEST(Segment2Tests, SquaredDistanceBeyondEndReproducesKnownUpstreamBug)
    {
        // See Segment2.hpp's header comment: for a point beyond End, upstream computes the
        // correct "distance to End" value in this branch but never returns it, so execution
        // falls through to the interior-projection formula instead.
        //
        // Segment (0,0)->(20,0), point (40,20). The mathematically CORRECT squared distance to
        // the nearer endpoint (End = (20,0)) is (40-20)^2 + (20-0)^2 = 800. The actual (buggy)
        // fallthrough formula instead computes the point's squared perpendicular distance to the
        // *infinite line* through Start/End, which is always <= the correct segment distance:
        // startToPointDistanceSquared(2000) - dot(800)^2 / startToEndDistanceSquared(400) = 400.
        // Upstream's own commented-out (never-run) test for this exact case recorded 400 too --
        // apparently captured from the actual buggy output, despite that test's own SetName
        // claiming it should be "the squared distance from the segment's ending point".
        const Segment2 segment(Vector2(0, 0), Vector2(20, 0));
        const float actual = segment.SquaredDistanceTo(Vector2(40, 20));
        constexpr float kMathematicallyCorrectDistanceToEnd = 800.0f;
        constexpr float kActualBuggyResult = 400.0f;
        EXPECT_FLOAT_EQ(actual, kActualBuggyResult);
        EXPECT_NE(actual, kMathematicallyCorrectDistanceToEnd)
            << "If this starts passing, upstream's bug may have been fixed and this port's "
               "fidelity note in Segment2.hpp should be revisited.";
    }

    TEST(Segment2Tests, DistanceToIsSqrtOfSquaredDistance)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(20, 0));
        EXPECT_FLOAT_EQ(segment.DistanceTo(Vector2(10, 25)), std::sqrt(625.0f));
    }

    TEST(Segment2Tests, EqualsAndOperators)
    {
        const Segment2 a(Vector2(0, 0), Vector2(50, 50));
        const Segment2 b(Vector2(0, 0), Vector2(50, 50));
        const Segment2 c(Vector2(50, 50), Vector2(50, 50));
        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
        EXPECT_TRUE(a.Equals(b));
    }

    TEST(Segment2Tests, GetHashCodeMatchesForEqualInstances)
    {
        const Segment2 a(Vector2(0, 0), Vector2(50, 50));
        const Segment2 b(Vector2(0, 0), Vector2(50, 50));
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(Segment2Tests, ToStringFormatsStartAndEnd)
    {
        const Segment2 segment(Vector2(0, 0), Vector2(0, 0));
        EXPECT_NE(segment.ToString().find("->"), std::string::npos);
    }
}
