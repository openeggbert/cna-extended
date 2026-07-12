// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingPolygon2DTest.cs,
// covering the subset of upstream test cases that don't require Collision2D, which is deferred
// -- see BoundingPolygon2D.hpp.
#include "CNA/Extended/BoundingPolygon2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"

#include <gtest/gtest.h>
#include <numbers>

namespace CNA::Extended
{
    namespace
    {
        std::vector<Vector2> Square()
        {
            return {Vector2(0, 0), Vector2(4, 0), Vector2(4, 4), Vector2(0, 4)};
        }
    }

    TEST(BoundingPolygon2DTests, ConstructorComputesNormals)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_EQ(polygon.getVertexCountProperty(), 4);
        EXPECT_EQ(polygon.Normals.size(), 4u);
    }

    TEST(BoundingPolygon2DTests, ConstructorThrowsOnFewerThanThreeVertices)
    {
        EXPECT_THROW(BoundingPolygon2D(std::vector<Vector2>{Vector2(0, 0), Vector2(1, 0)}), std::invalid_argument);
    }

    TEST(BoundingPolygon2DTests, ConstructorWithNormalsThrowsOnLengthMismatch)
    {
        EXPECT_THROW(BoundingPolygon2D(Square(), std::vector<Vector2>{Vector2(0, 1)}), std::invalid_argument);
    }

    TEST(BoundingPolygon2DTests, VertexCount)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_EQ(polygon.getVertexCountProperty(), 4);
    }

    TEST(BoundingPolygon2DTests, CentroidOfSquareIsCenter)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_EQ(polygon.getCentroidProperty(), Vector2(2, 2));
    }

    TEST(BoundingPolygon2DTests, AreaOfSquare)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_FLOAT_EQ(polygon.getAreaProperty(), 16.0f);
    }

    TEST(BoundingPolygon2DTests, CreateFromVertices)
    {
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateFromVertices(Square());
        EXPECT_EQ(polygon.getVertexCountProperty(), 4);
    }

    TEST(BoundingPolygon2DTests, CreateRegularProducesCorrectSideCount)
    {
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateRegular(Vector2::Zero, 5.0f, 6);
        EXPECT_EQ(polygon.getVertexCountProperty(), 6);

        for (const Vector2& v : polygon.Vertices)
        {
            EXPECT_NEAR(v.Length(), 5.0f, 1e-3f);
        }
    }

    TEST(BoundingPolygon2DTests, CreateRegularThrowsOnFewerThanThreeSides)
    {
        EXPECT_THROW((void)BoundingPolygon2D::CreateRegular(Vector2::Zero, 5.0f, 2), std::invalid_argument);
    }

    TEST(BoundingPolygon2DTests, CreateFromBoundingBox2DProducesFourVertices)
    {
        const BoundingBox2D box(Vector2(0, 0), Vector2(4, 4));
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateFromBoundingBox2D(box);

        EXPECT_EQ(polygon.getVertexCountProperty(), 4);
        EXPECT_FLOAT_EQ(polygon.getAreaProperty(), 16.0f);
    }

    TEST(BoundingPolygon2DTests, CreateMergedEnclosesBothPolygons)
    {
        const BoundingPolygon2D a(Square());
        std::vector<Vector2> secondSquare = {Vector2(3, 3), Vector2(7, 3), Vector2(7, 7), Vector2(3, 7)};
        const BoundingPolygon2D b(secondSquare);

        const BoundingPolygon2D merged = BoundingPolygon2D::CreateMerged(a, b);

        // The convex hull of two overlapping squares must have a larger area than either alone.
        EXPECT_GT(merged.getAreaProperty(), a.getAreaProperty());
        EXPECT_GT(merged.getAreaProperty(), b.getAreaProperty());
    }

    TEST(BoundingPolygon2DTests, CreateMergedWithEmptyReturnsTheOther)
    {
        const BoundingPolygon2D empty;
        const BoundingPolygon2D square(Square());

        EXPECT_EQ(BoundingPolygon2D::CreateMerged(empty, square), square);
        EXPECT_EQ(BoundingPolygon2D::CreateMerged(square, empty), square);
    }

    TEST(BoundingPolygon2DTests, TranslateMovesAllVertices)
    {
        const BoundingPolygon2D polygon(Square());
        const BoundingPolygon2D translated = polygon.Translate(Vector2(1, 1));

        EXPECT_EQ(translated.Vertices[0], Vector2(1, 1));
        EXPECT_EQ(translated.Vertices[2], Vector2(5, 5));
        EXPECT_EQ(translated.Normals, polygon.Normals);
    }

    TEST(BoundingPolygon2DTests, DeconstructReturnsVerticesAndNormals)
    {
        const BoundingPolygon2D polygon(Square());
        std::vector<Vector2> vertices, normals;
        polygon.Deconstruct(vertices, normals);

        EXPECT_EQ(vertices, polygon.Vertices);
        EXPECT_EQ(normals, polygon.Normals);
    }

    TEST(BoundingPolygon2DTests, EqualsAndOperators)
    {
        const BoundingPolygon2D a(Square());
        const BoundingPolygon2D b(Square());
        std::vector<Vector2> differentShape = {Vector2(0, 0), Vector2(5, 0), Vector2(5, 5), Vector2(0, 5)};
        const BoundingPolygon2D c(differentShape);

        EXPECT_TRUE(a == b);
        EXPECT_TRUE(a != c);
    }

    TEST(BoundingPolygon2DTests, GetHashCodeMatchesForEqualInstances)
    {
        const BoundingPolygon2D a(Square());
        const BoundingPolygon2D b(Square());

        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(BoundingPolygon2DTests, GetHashCodeOfDefaultConstructedIsZero)
    {
        const BoundingPolygon2D empty;
        EXPECT_EQ(empty.GetHashCode(), 0);
    }

    TEST(BoundingPolygon2DTests, DefaultConstructedIsEmpty)
    {
        const BoundingPolygon2D polygon;
        EXPECT_EQ(polygon.getVertexCountProperty(), 0);
        EXPECT_EQ(polygon.getCentroidProperty(), Vector2::Zero);
        EXPECT_FLOAT_EQ(polygon.getAreaProperty(), 0.0f);
    }
}
