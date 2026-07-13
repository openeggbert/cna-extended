// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for Triangle.cs. These tests cover
// ContainsPoint (instance + static), Equals/GetHashCode, and operators directly.
#include "CNA/Extended/Triangulation/Triangle.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    TEST(TriangleTests, ContainsPointInside)
    {
        const Triangle triangle(Vertex(Vector2(0, 0), 0), Vertex(Vector2(10, 0), 1), Vertex(Vector2(0, 10), 2));
        EXPECT_TRUE(triangle.ContainsPoint(Vertex(Vector2(2, 2), 3)));
    }

    TEST(TriangleTests, ContainsPointOutside)
    {
        const Triangle triangle(Vertex(Vector2(0, 0), 0), Vertex(Vector2(10, 0), 1), Vertex(Vector2(0, 10), 2));
        EXPECT_FALSE(triangle.ContainsPoint(Vertex(Vector2(20, 20), 3)));
    }

    TEST(TriangleTests, ContainsPointAtVertexIsTrue)
    {
        const Vertex a(Vector2(0, 0), 0);
        const Triangle triangle(a, Vertex(Vector2(10, 0), 1), Vertex(Vector2(0, 10), 2));
        EXPECT_TRUE(triangle.ContainsPoint(a));
    }

    TEST(TriangleTests, StaticContainsPointMatchesInstanceOverload)
    {
        const Vertex a(Vector2(0, 0), 0);
        const Vertex b(Vector2(10, 0), 1);
        const Vertex c(Vector2(0, 10), 2);
        const Vertex point(Vector2(2, 2), 3);

        EXPECT_EQ(Triangle::ContainsPoint(a, b, c, point), Triangle(a, b, c).ContainsPoint(point));
    }

    TEST(TriangleTests, EqualsAndOperators)
    {
        const Vertex a(Vector2(0, 0), 0);
        const Vertex b(Vector2(10, 0), 1);
        const Vertex c(Vector2(0, 10), 2);

        const Triangle first(a, b, c);
        const Triangle second(a, b, c);
        const Triangle third(a, b, Vertex(Vector2(0, 20), 2));

        EXPECT_TRUE(first == second);
        EXPECT_TRUE(first != third);
        EXPECT_TRUE(first.Equals(second));
    }

    TEST(TriangleTests, GetHashCodeMatchesForEqualTriangles)
    {
        const Vertex a(Vector2(0, 0), 0);
        const Vertex b(Vector2(10, 0), 1);
        const Vertex c(Vector2(0, 10), 2);

        const Triangle first(a, b, c);
        const Triangle second(a, b, c);
        EXPECT_EQ(first.GetHashCode(), second.GetHashCode());
    }
}
