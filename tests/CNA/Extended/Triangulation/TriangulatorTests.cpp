// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// The DetermineWindingOrder tests are ported 1:1 from MonoGame.Extended's
// tests/MonoGame.Extended.Tests/Math/TriangulatorTests.cs (the only upstream test file for this
// module, and unlike several other Math/Primitives test files elsewhere in this project's
// history, this one is actively run upstream, not commented out). Triangulate/CutHoleInShape/
// EnsureWindingOrder/ReverseWindingOrder have no upstream coverage -- fresh tests added.
#include "CNA/Extended/Triangulation/Triangulator.hpp"

#include <cmath>
#include <numbers>

#include <gtest/gtest.h>

namespace CNA::Extended::Triangulation
{
    namespace
    {
        // Simple clockwise square (mathematical convention, Y-up).
        std::vector<Vector2> CwSquare()
        {
            return {Vector2(0, 0), Vector2(0, 1), Vector2(1, 1), Vector2(1, 0)};
        }

        // Simple counter-clockwise square.
        std::vector<Vector2> CcwSquare()
        {
            return {Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1)};
        }

        // Creates a 5-point star polygon with alternating outer and inner vertices. A star has
        // exactly 5 clockwise turns and 5 counter-clockwise turns, which caused the old
        // angle-counting algorithm to produce incorrect results (upstream issue #791).
        std::vector<Vector2> CreateStarVertices(const float outerRadius = 100.0f, const float innerRadius = 40.0f)
        {
            std::vector<Vector2> vertices(10);
            for (int i = 0; i < 5; i++)
            {
                const float outerAngle = static_cast<float>(std::numbers::pi / 2.0 + i * 2.0 * std::numbers::pi / 5.0);
                const float innerAngle = outerAngle + static_cast<float>(std::numbers::pi / 5.0);
                vertices[static_cast<std::size_t>(i * 2)] = Vector2(outerRadius * std::cos(outerAngle), outerRadius * std::sin(outerAngle));
                vertices[static_cast<std::size_t>(i * 2 + 1)] = Vector2(innerRadius * std::cos(innerAngle), innerRadius * std::sin(innerAngle));
            }
            return vertices;
        }

        std::vector<Vector2> Reverse(const std::vector<Vector2>& vertices)
        {
            std::vector<Vector2> reversed(vertices.size());
            reversed[0] = vertices[0];
            for (std::size_t i = 1; i < vertices.size(); i++)
            {
                reversed[i] = vertices[vertices.size() - i];
            }
            return reversed;
        }
    }

    TEST(TriangulatorTests, DetermineWindingOrderClockwiseSquareReturnsClockwise)
    {
        EXPECT_EQ(Triangulator::DetermineWindingOrder(CwSquare()), WindingOrder::Clockwise);
    }

    TEST(TriangulatorTests, DetermineWindingOrderCounterClockwiseSquareReturnsCounterClockwise)
    {
        EXPECT_EQ(Triangulator::DetermineWindingOrder(CcwSquare()), WindingOrder::CounterClockwise);
    }

    TEST(TriangulatorTests, DetermineWindingOrderReversedPolygonReturnsOppositeOrder)
    {
        EXPECT_NE(Triangulator::DetermineWindingOrder(CwSquare()), Triangulator::DetermineWindingOrder(Reverse(CwSquare())));
    }

    // Regression test for upstream issue #791. A 5-point star has equal CW and CCW turns,
    // causing the old angle-counting algorithm to return the same winding order for both the
    // star and its reverse. The shoelace formula fixes this.
    TEST(TriangulatorTests, DetermineWindingOrderStarPolygonReturnsOppositeForReverse)
    {
        const std::vector<Vector2> star = CreateStarVertices();
        const std::vector<Vector2> starReversed = Reverse(star);

        EXPECT_NE(Triangulator::DetermineWindingOrder(star), Triangulator::DetermineWindingOrder(starReversed));
    }

    TEST(TriangulatorTests, EnsureWindingOrderReversesWhenOrderDoesNotMatch)
    {
        const std::vector<Vector2> square = CcwSquare();
        const std::vector<Vector2> result = Triangulator::EnsureWindingOrder(square, WindingOrder::Clockwise);
        EXPECT_EQ(Triangulator::DetermineWindingOrder(result), WindingOrder::Clockwise);
    }

    TEST(TriangulatorTests, EnsureWindingOrderLeavesMatchingOrderUnchanged)
    {
        const std::vector<Vector2> square = CcwSquare();
        const std::vector<Vector2> result = Triangulator::EnsureWindingOrder(square, WindingOrder::CounterClockwise);
        EXPECT_EQ(result, square);
    }

    TEST(TriangulatorTests, ReverseWindingOrderRoundTrips)
    {
        const std::vector<Vector2> square = CcwSquare();
        const std::vector<Vector2> reversedTwice = Triangulator::ReverseWindingOrder(Triangulator::ReverseWindingOrder(square));
        EXPECT_EQ(reversedTwice, square);
    }

    TEST(TriangulatorTests, TriangulateConvexSquareProducesTwoTriangles)
    {
        std::vector<Vector2> outputVertices;
        std::vector<int> indices;
        Triangulator::Triangulate(CcwSquare(), WindingOrder::CounterClockwise, outputVertices, indices);

        EXPECT_EQ(outputVertices.size(), 4u);
        EXPECT_EQ(indices.size(), 6u); // 2 triangles * 3 indices
        for (const int index : indices)
        {
            EXPECT_GE(index, 0);
            EXPECT_LT(static_cast<std::size_t>(index), outputVertices.size());
        }
    }

    TEST(TriangulatorTests, TriangulateReflexPolygonProducesExpectedTriangleCount)
    {
        // An "L-shaped" hexagon with one reflex vertex at (1, 1).
        const std::vector<Vector2> lShape = {
            Vector2(0, 0), Vector2(2, 0), Vector2(2, 1), Vector2(1, 1), Vector2(1, 2), Vector2(0, 2),
        };

        std::vector<Vector2> outputVertices;
        std::vector<int> indices;
        Triangulator::Triangulate(lShape, WindingOrder::CounterClockwise, outputVertices, indices);

        // n-2 triangles for a simple polygon with n vertices.
        EXPECT_EQ(indices.size(), (lShape.size() - 2) * 3);
    }

    TEST(TriangulatorTests, TriangulateRespectsDesiredWindingOrder)
    {
        std::vector<Vector2> ccwOutputVertices;
        std::vector<int> ccwIndices;
        Triangulator::Triangulate(CcwSquare(), WindingOrder::CounterClockwise, ccwOutputVertices, ccwIndices);

        std::vector<Vector2> cwOutputVertices;
        std::vector<int> cwIndices;
        Triangulator::Triangulate(CcwSquare(), WindingOrder::Clockwise, cwOutputVertices, cwIndices);

        // Same triangle set, opposite index order within each triangle (A,B,C vs C,B,A).
        ASSERT_EQ(ccwIndices.size(), cwIndices.size());
        for (std::size_t i = 0; i < ccwIndices.size(); i += 3)
        {
            EXPECT_EQ(ccwIndices[i], cwIndices[i + 2]);
            EXPECT_EQ(ccwIndices[i + 1], cwIndices[i + 1]);
            EXPECT_EQ(ccwIndices[i + 2], cwIndices[i]);
        }
    }

    TEST(TriangulatorTests, CutHoleInShapeInsertsHoleVerticesIntoShape)
    {
        const std::vector<Vector2> shape = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)};
        const std::vector<Vector2> hole = {Vector2(4, 4), Vector2(6, 4), Vector2(6, 6), Vector2(4, 6)};

        const std::vector<Vector2> result = Triangulator::CutHoleInShape(shape, hole);

        // The algorithm injects holePolygon.Count + 2 additional vertices into the shape.
        EXPECT_EQ(result.size(), shape.size() + hole.size() + 2);
    }

    TEST(TriangulatorTests, CutHoleInShapeResultIsTriangulatable)
    {
        const std::vector<Vector2> shape = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)};
        const std::vector<Vector2> hole = {Vector2(4, 4), Vector2(6, 4), Vector2(6, 6), Vector2(4, 6)};

        const std::vector<Vector2> shapeWithHole = Triangulator::CutHoleInShape(shape, hole);

        std::vector<Vector2> outputVertices;
        std::vector<int> indices;
        Triangulator::Triangulate(shapeWithHole, WindingOrder::CounterClockwise, outputVertices, indices);

        EXPECT_FALSE(indices.empty());
        EXPECT_EQ(indices.size() % 3, 0u);
    }
}
