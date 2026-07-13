// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Translated from MonoGame.Extended's tests/MonoGame.Extended.Tests/BoundingPolygon2DTest.cs.
// The ContainsPoint and TryGetCollision regions were originally deferred pending Collision2D --
// see BoundingPolygon2D.hpp -- and are now ported alongside the methods they cover.
#include "CNA/Extended/BoundingPolygon2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>
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

        BoundingPolygon2D CreateRectanglePolygon(const Vector2& min, const Vector2& max)
        {
            std::vector<Vector2> vertices = {
                Vector2(min.X, min.Y),
                Vector2(max.X, min.Y),
                Vector2(max.X, max.Y),
                Vector2(min.X, max.Y),
            };
            std::vector<Vector2> normals = {
                -Vector2::UnitY,
                Vector2::UnitX,
                Vector2::UnitY,
                -Vector2::UnitX,
            };

            return BoundingPolygon2D(std::move(vertices), std::move(normals));
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

    TEST(BoundingPolygon2DTests, ConstructorWithExplicitNormals)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10)};
        const std::vector<Vector2> normals = {Vector2(0, -1), Vector2(1, 0), Vector2(-0.707f, 0.707f)};
        const BoundingPolygon2D polygon(vertices, normals);

        EXPECT_EQ(polygon.getVertexCountProperty(), 3);
    }

    TEST(BoundingPolygon2DTests, VertexCount)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_EQ(polygon.getVertexCountProperty(), 4);
    }

    TEST(BoundingPolygon2DTests, CentroidOfTriangle)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(5, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Vector2 centroid = polygon.getCentroidProperty();

        EXPECT_NEAR(centroid.X, 5.0f, 1e-4f);
        EXPECT_NEAR(centroid.Y, 10.0f / 3.0f, 1e-4f);
    }

    TEST(BoundingPolygon2DTests, CentroidOfSquareIsCenter)
    {
        const BoundingPolygon2D polygon(Square());
        EXPECT_EQ(polygon.getCentroidProperty(), Vector2(2, 2));
    }

    TEST(BoundingPolygon2DTests, AreaOfTriangle)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(5, 10)};
        const BoundingPolygon2D polygon(vertices);
        EXPECT_NEAR(polygon.getAreaProperty(), 50.0f, 1e-4f);
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

    TEST(BoundingPolygon2DTests, CreateRegularTriangleVertexDistancesMatchRadius)
    {
        const Vector2 center(5, 5);
        const float radius = 10.0f;
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateRegular(center, radius, 3);

        EXPECT_EQ(polygon.getVertexCountProperty(), 3);
        for (const Vector2& v : polygon.Vertices)
        {
            EXPECT_NEAR(Vector2::Distance(center, v), radius, 1e-3f);
        }
    }

    TEST(BoundingPolygon2DTests, CreateRegularSquareHasFourVertices)
    {
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateRegular(Vector2::Zero, 10.0f, 4);
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

    TEST(BoundingPolygon2DTests, CreateRegularWithRotation)
    {
        const float radius = 10.0f;
        const float rotation = Microsoft::Xna::Framework::MathHelper::PiOver4;
        const BoundingPolygon2D polygon = BoundingPolygon2D::CreateRegular(Vector2::Zero, radius, 4, rotation);

        EXPECT_EQ(polygon.getVertexCountProperty(), 4);

        const float expectedX = radius * std::cos(rotation);
        const float expectedY = radius * std::sin(rotation);
        EXPECT_NEAR(polygon.Vertices[0].X, expectedX, 1e-4f);
        EXPECT_NEAR(polygon.Vertices[0].Y, expectedY, 1e-4f);
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

    TEST(BoundingPolygon2DTests, TransformTranslation)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Matrix matrix = Matrix::CreateTranslation(5, 10, 0);
        const BoundingPolygon2D transformed = polygon.Transform(matrix);

        EXPECT_EQ(transformed.Vertices[0], Vector2(5, 10));
        EXPECT_EQ(transformed.Vertices[1], Vector2(15, 10));
        EXPECT_EQ(transformed.Vertices[2], Vector2(15, 20));
    }

    TEST(BoundingPolygon2DTests, TransformUniformScale)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Matrix matrix = Matrix::CreateScale(2.0f);
        const BoundingPolygon2D transformed = polygon.Transform(matrix);

        EXPECT_EQ(transformed.Vertices[0], Vector2(0, 0));
        EXPECT_EQ(transformed.Vertices[1], Vector2(20, 0));
        EXPECT_EQ(transformed.Vertices[2], Vector2(20, 20));
    }

    TEST(BoundingPolygon2DTests, TransformRotation)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(0, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Matrix matrix = Matrix::CreateRotationZ(Microsoft::Xna::Framework::MathHelper::PiOver2);
        const BoundingPolygon2D transformed = polygon.Transform(matrix);

        constexpr float tolerance = 1e-4f;
        EXPECT_NEAR(transformed.Vertices[0].X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Vertices[0].Y, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Vertices[1].X, 0.0f, tolerance);
        EXPECT_NEAR(transformed.Vertices[1].Y, 10.0f, tolerance);
        EXPECT_NEAR(transformed.Vertices[2].X, -10.0f, tolerance);
        EXPECT_NEAR(transformed.Vertices[2].Y, 0.0f, tolerance);
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

    TEST(BoundingPolygon2DTests, ContainsPointInside)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Vector2 point(5, 5);

        const ContainmentType result = polygon.Contains(point);

        EXPECT_EQ(result, ContainmentType::Contains);
    }

    TEST(BoundingPolygon2DTests, ContainsPointOnBoundary)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Vector2 point(10, 5);

        const ContainmentType result = polygon.Contains(point);

        EXPECT_EQ(result, ContainmentType::Contains);
    }

    TEST(BoundingPolygon2DTests, ContainsPointOutside)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)};
        const BoundingPolygon2D polygon(vertices);
        const Vector2 point(15, 5);

        const ContainmentType result = polygon.Contains(point);

        EXPECT_EQ(result, ContainmentType::Disjoint);
    }

    TEST(BoundingPolygon2DTests, TryGetCollisionWithBoxReturnsReceiverMinimumTranslationVector)
    {
        const BoundingPolygon2D polygon = CreateRectanglePolygon(Vector2(1.0f, -2.0f), Vector2(5.0f, 2.0f));
        const BoundingBox2D box(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = polygon.TryGetCollision(box, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(1.0f, 0.0f));
    }

    TEST(BoundingPolygon2DTests, TryGetCollisionWithOrientedBoxReturnsReceiverMinimumTranslationVector)
    {
        const BoundingPolygon2D polygon = CreateRectanglePolygon(Vector2(1.0f, -2.0f), Vector2(5.0f, 2.0f));
        const OrientedBoundingBox2D obb(Vector2::Zero, Vector2::UnitX, Vector2::UnitY, Vector2(2.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = polygon.TryGetCollision(obb, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(1.0f, 0.0f));
    }

    TEST(BoundingPolygon2DTests, TryGetCollisionWithPolygonReturnsReceiverMinimumTranslationVector)
    {
        const BoundingPolygon2D polygon = CreateRectanglePolygon(Vector2(-2.0f, -2.0f), Vector2(2.0f, 2.0f));
        const BoundingPolygon2D other = CreateRectanglePolygon(Vector2(1.0f, -2.0f), Vector2(5.0f, 2.0f));

        CollisionResult2D result;
        const bool intersects = polygon.TryGetCollision(other, result);

        EXPECT_TRUE(intersects);
        EXPECT_TRUE(result.Intersects);
        EXPECT_EQ(result.Normal, -Vector2::UnitX);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, 1.0f);
        EXPECT_EQ(result.MinimumTranslationVector, Vector2(-1.0f, 0.0f));
    }

    TEST(BoundingPolygon2DTests, TryGetCollisionWithSeparatedPolygonReturnsFalseAndNone)
    {
        const BoundingPolygon2D polygon = CreateRectanglePolygon(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
        const BoundingPolygon2D other = CreateRectanglePolygon(Vector2(4.0f, -1.0f), Vector2(6.0f, 1.0f));

        CollisionResult2D result;
        const bool intersects = polygon.TryGetCollision(other, result);

        EXPECT_FALSE(intersects);
        EXPECT_FALSE(result.Intersects);
        EXPECT_EQ(result.Normal, CollisionResult2D::None.Normal);
        EXPECT_FLOAT_EQ(result.PenetrationDepth, CollisionResult2D::None.PenetrationDepth);
        EXPECT_EQ(result.MinimumTranslationVector, CollisionResult2D::None.MinimumTranslationVector);
    }
}
