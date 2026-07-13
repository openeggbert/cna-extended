// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported 1:1 from MonoGame.Extended's tests/MonoGame.Extended.Tests/Shapes/PolygonTests.cs.
#include "CNA/Extended/Shapes/Polygon.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended::Shapes
{
    using Microsoft::Xna::Framework::MathHelper;

    TEST(PolygonTests, PolygonContainsPointTest)
    {
        const std::vector<Vector2> vertices = {
            Vector2(0, 0),
            Vector2(10, 0),
            Vector2(10, 10),
            Vector2(0, 10),
        };

        const Polygon polygon(vertices);

        EXPECT_TRUE(polygon.Contains(Vector2(5, 5)));
        EXPECT_TRUE(polygon.Contains(Vector2(0.01f, 0.01f)));
        EXPECT_TRUE(polygon.Contains(Vector2(9.99f, 9.99f)));
        EXPECT_FALSE(polygon.Contains(Vector2(-1.0f, -1.0f)));
        EXPECT_FALSE(polygon.Contains(Vector2(-11.0f, -11.0f)));

        // Reference: https://github.com/craftworkgames/MonoGame.Extended/issues/214
        // To maintain consistency with behavior in MonoGame, a point that exists at the edge of a
        // polygon is -not- contained within the polygon and should return false.
        EXPECT_FALSE(polygon.Contains(Vector2(10.0f, 10.0f)));
    }

    TEST(PolygonTests, PolygonTransformTranslationTest)
    {
        const std::vector<Vector2> vertices = {
            Vector2(0, 0),
            Vector2(10, 0),
            Vector2(10, 10),
            Vector2(0, 10),
        };

        Polygon polygon(vertices);
        polygon.Offset(Vector2(2, 3));

        const auto& transformed = polygon.getVerticesProperty();
        EXPECT_EQ(transformed[0], Vector2(2, 3));
        EXPECT_EQ(transformed[1], Vector2(12, 3));
        EXPECT_EQ(transformed[2], Vector2(12, 13));
        EXPECT_EQ(transformed[3], Vector2(2, 13));
    }

    TEST(PolygonTests, PolygonTransformRotationTest)
    {
        const std::vector<Vector2> vertices = {
            Vector2(-5, -5),
            Vector2(5, 10),
            Vector2(-5, 10),
        };

        Polygon polygon(vertices);
        polygon.Rotate(MathHelper::ToRadians(90.0f));

        constexpr float tolerance = 0.01f;
        const auto& transformed = polygon.getVerticesProperty();
        EXPECT_NEAR(transformed[0].X, 5.0f, tolerance);
        EXPECT_NEAR(transformed[0].Y, -5.0f, tolerance);
        EXPECT_NEAR(transformed[1].X, -10.0f, tolerance);
        EXPECT_NEAR(transformed[1].Y, 5.0f, tolerance);
        EXPECT_NEAR(transformed[2].X, -10.0f, tolerance);
        EXPECT_NEAR(transformed[2].Y, -5.0f, tolerance);
    }

    TEST(PolygonTests, PolygonTransformScaleTest)
    {
        const std::vector<Vector2> vertices = {
            Vector2(0, -1),
            Vector2(1, 1),
            Vector2(-1, 1),
        };

        Polygon polygon(vertices);
        polygon.Scale(Vector2(1, -0.5f));

        constexpr float tolerance = 0.01f;
        const auto& transformed = polygon.getVerticesProperty();
        EXPECT_NEAR(transformed[0].X, 0.0f, tolerance);
        EXPECT_NEAR(transformed[0].Y, -0.5f, tolerance);
        EXPECT_NEAR(transformed[1].X, 2.0f, tolerance);
        EXPECT_NEAR(transformed[1].Y, 0.5f, tolerance);
        EXPECT_NEAR(transformed[2].X, -2.0f, tolerance);
        EXPECT_NEAR(transformed[2].Y, 0.5f, tolerance);
    }

    TEST(PolygonTests, GetHashCodeMatchesForEqualPolygons)
    {
        const std::vector<Vector2> vertices = {Vector2(0, 0), Vector2(1, 0), Vector2(1, 1)};
        const Polygon a(vertices);
        const Polygon b(vertices);
        EXPECT_EQ(a.GetHashCode(), b.GetHashCode());
    }

    TEST(PolygonTests, BoundingRectangleEnclosesVertices)
    {
        const std::vector<Vector2> vertices = {Vector2(1, 2), Vector2(5, 2), Vector2(5, 8), Vector2(1, 8)};
        const Polygon polygon(vertices);
        const RectangleF bounds = polygon.getBoundingRectangleProperty();
        EXPECT_FLOAT_EQ(bounds.X, 1.0f);
        EXPECT_FLOAT_EQ(bounds.Y, 2.0f);
        EXPECT_FLOAT_EQ(bounds.Width, 4.0f);
        EXPECT_FLOAT_EQ(bounds.Height, 6.0f);
    }
}
