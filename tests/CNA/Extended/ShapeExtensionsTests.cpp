// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// No upstream tests exist for ShapeExtensions. Fresh tests below. A plain
// `GraphicsDevice`/`SpriteBatch`/`Texture2D(GraphicsDevice&, w, h)` triple genuinely
// constructs and renders end-to-end headlessly in this environment (real EasyGL-over-Mesa
// software rendering, confirmed this session) -- so these tests exercise real
// Begin/Draw/End calls, not just compile-checks.
#include "CNA/Extended/ShapeExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"

#include <gtest/gtest.h>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Graphics::GraphicsDevice;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;
    using Microsoft::Xna::Framework::Graphics::Viewport;

    namespace
    {
        class ShapeExtensionsTest : public ::testing::Test
        {
        protected:
            void SetUp() override { graphicsDevice.setViewportProperty(Viewport(0, 0, 800, 480)); }

            GraphicsDevice graphicsDevice;
        };
    }

    TEST_F(ShapeExtensionsTest, DrawPolygonFromShape_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        const Shapes::Polygon polygon({Vector2(0, 0), Vector2(10, 0), Vector2(10, 10), Vector2(0, 10)});

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawPolygon(spriteBatch, Vector2(5, 5), polygon, Color::Red));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawPolygonFromPoints_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Vector2> points{Vector2(0, 0), Vector2(20, 0), Vector2(10, 20)};

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawPolygon(spriteBatch, Vector2::Zero, points, Color::Blue, 2.0f));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawPolygonFromPoints_SinglePoint_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        const std::vector<Vector2> points{Vector2(5, 5)};

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawPolygon(spriteBatch, Vector2::Zero, points, Color::Green));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, FillRectangle_RectangleFOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(FillRectangle(spriteBatch, RectangleF(0, 0, 50, 30), Color::Yellow));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, FillRectangle_LocationSizeOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(FillRectangle(spriteBatch, Vector2(10, 10), SizeF(40.0f, 20.0f), Color::Cyan));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, FillRectangle_XywhOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(FillRectangle(spriteBatch, 0.0f, 0.0f, 15.0f, 15.0f, Color::White));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawRectangle_RectangleFOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawRectangle(spriteBatch, RectangleF(0, 0, 50, 30), Color::Black, 2.0f));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawRectangle_LocationSizeOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawRectangle(spriteBatch, Vector2(0, 0), SizeF(30.0f, 30.0f), Color::Orange));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawRectangle_XywhOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawRectangle(spriteBatch, 0.0f, 0.0f, 25.0f, 25.0f, Color::Purple));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawLine_XyOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawLine(spriteBatch, 0.0f, 0.0f, 100.0f, 100.0f, Color::Red));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawLine_PointOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawLine(spriteBatch, Vector2(0, 0), Vector2(50, 50), Color::Blue, 3.0f));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawLine_LengthAngleOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawLine(spriteBatch, Vector2(10, 10), 40.0f, 0.5f, Color::Green));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawPoint_XyOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawPoint(spriteBatch, 5.0f, 5.0f, Color::White, 3.0f));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawPoint_PositionOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawPoint(spriteBatch, Vector2(5, 5), Color::White));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawCircle_CircleFOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);
        const CircleF circle(Vector2(50, 50), 20.0f);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawCircle(spriteBatch, circle, 16, Color::Red));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawCircle_CenterRadiusOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawCircle(spriteBatch, Vector2(0, 0), 10.0f, 8, Color::Blue));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawCircle_XyOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawCircle(spriteBatch, 0.0f, 0.0f, 10.0f, 8, Color::Green));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawEllipse_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawEllipse(spriteBatch, Vector2(0, 0), Vector2(20, 10), 16, Color::Yellow));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawArc_CenterOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawArc(spriteBatch, Vector2(0, 0), 15.0f, 0.0f, 3.14159f, 8, Color::Cyan));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, DrawArc_XyOverload_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        EXPECT_NO_THROW(DrawArc(spriteBatch, 0.0f, 0.0f, 15.0f, 0.0f, 3.14159f, 8, Color::Magenta));
        spriteBatch.End();
    }

    TEST_F(ShapeExtensionsTest, MultipleShapes_SameBatch_DoesNotThrow)
    {
        SpriteBatch spriteBatch(graphicsDevice);

        spriteBatch.Begin();
        DrawRectangle(spriteBatch, RectangleF(0, 0, 10, 10), Color::Red);
        FillRectangle(spriteBatch, RectangleF(20, 0, 10, 10), Color::Green);
        DrawCircle(spriteBatch, Vector2(50, 50), 5.0f, 6, Color::Blue);
        DrawLine(spriteBatch, Vector2(0, 0), Vector2(50, 50), Color::White);
        EXPECT_NO_THROW(spriteBatch.End());
    }
}
