// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// MonoGame.Extended has no dedicated upstream tests for Vertex.cs. These tests cover
// construction, Equals/GetHashCode/ToString, and operators directly.
#include "CNA/Extended/Triangulation/Vertex.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    TEST(VertexTests, ConstructorSetsPositionAndIndex)
    {
        const Vertex vertex(Vector2(1, 2), 5);
        EXPECT_EQ(vertex.Position, Vector2(1, 2));
        EXPECT_EQ(vertex.Index, 5);
    }

    TEST(VertexTests, EqualsAndOperators)
    {
        const Vertex a(Vector2(1, 2), 5);
        const Vertex b(Vector2(1, 2), 5);
        const Vertex c(Vector2(1, 2), 6);
        const Vertex d(Vector2(3, 4), 5);

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a.Equals(b));
        EXPECT_TRUE(a != c);
        EXPECT_TRUE(a != d);
    }

    TEST(VertexTests, GetHashCodeMatchesForEqualVertices)
    {
        const Vertex a(Vector2(1, 2), 5);
        const Vertex b(Vector2(1, 2), 5);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(VertexTests, ToStringIncludesPositionAndIndex)
    {
        const Vertex vertex(Vector2(1, 2), 5);
        const std::string text = vertex.ToString();
        EXPECT_NE(text.find("5"), std::string::npos);
    }
}
