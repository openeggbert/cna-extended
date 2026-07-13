// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's VectorDraw/PrimitiveDrawing.cs (itself adapted upstream from
// Velcro/Farseer Physics, used with permission -- see upstream's own file header). Draws primitive
// shapes (points, lines, rectangles, circles, arcs, polygons, ellipses) using a PrimitiveBatch.
//
// `Vector2[] vertices` parameters -> `const std::vector<Vector2>&`, matching this project's
// established convention. Delegates polygon triangulation to CNA::Extended::Triangulation::
// Triangulator (already ported in Phase 1), matching upstream's own dependency on
// MonoGame.Extended.Triangulation.Triangulator -- not re-derived.
#pragma once

#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace CNA::Extended::VectorDraw
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector2;

    class PrimitiveBatch;

    /**
     * @brief Provides methods for drawing primitive shapes using a PrimitiveBatch.
     * @remark Call PrimitiveBatch::Begin() on the associated PrimitiveBatch before issuing any
     * draw calls, and PrimitiveBatch::End() when finished to flush geometry to the GPU.
     */
    class PrimitiveDrawing
    {
    public:
        /** @brief The default number of segments used when approximating circles and ellipses. */
        static constexpr int CircleSegments = 32;

        /**
         * @brief Initializes a new PrimitiveDrawing.
         * @param primitiveBatch The PrimitiveBatch used to issue draw calls. Must outlive this instance.
         */
        explicit PrimitiveDrawing(PrimitiveBatch& primitiveBatch);

        /** @brief Draws a single point at the specified position. */
        void DrawPoint(const Vector2& center, const Color& color);

        /** @brief Draws a rectangle outline. */
        void DrawRectangle(const Vector2& location, float width, float height, const Color& color);

        /**
         * @brief Draws a solid (filled) rectangle with an optional outline.
         * @param outline When true, an outline is drawn over the filled rectangle.
         */
        void DrawSolidRectangle(const Vector2& location, float width, float height, const Color& color, bool outline = true);

        /** @brief Draws a circle outline using CircleSegments segments. */
        void DrawCircle(const Vector2& center, float radius, const Color& color);

        /**
         * @brief Draws a solid (filled) circle with an optional outline using CircleSegments
         * segments. The fill and outline use the same color.
         */
        void DrawSolidCircle(const Vector2& center, float radius, const Color& color, bool outline = true);

        /** @brief Draws a solid (filled) circle with an optional outline using CircleSegments segments. */
        void DrawSolidCircle(const Vector2& center, float radius, const Color& color, const Color& fillColor, bool outline = true);

        /**
         * @brief Draws an arc outline.
         * @param sweepAngle The sweep angle in radians. Positive values sweep counter-clockwise.
         * Use MathHelper::TwoPi to draw a full circle.
         * @param sides The number of line segments used to approximate the arc.
         */
        void DrawArc(const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color);

        /** @brief Draws a solid (filled) arc (pie slice) with an outline. The fill and outline use the same color. */
        void DrawSolidArc(const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color);

        /** @brief Draws a solid (filled) arc (pie slice) with an outline. */
        void DrawSolidArc(
            const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color, const Color& fillColor);

        /** @brief Draws a line segment between two points. */
        void DrawSegment(const Vector2& start, const Vector2& end, const Color& color);

        /**
         * @brief Draws a polygon outline.
         * @param position The world offset applied to all vertices.
         * @param vertices The polygon vertices in local space.
         * @param closed When true, an edge is drawn between the last and first vertex to close the polygon.
         */
        void DrawPolygon(const Vector2& position, const std::vector<Vector2>& vertices, const Color& color, bool closed = true);

        /**
         * @brief Draws a solid (filled) polygon with an optional outline.
         * @param outline When true, an outline is drawn over the filled polygon.
         */
        void DrawSolidPolygon(const Vector2& position, const std::vector<Vector2>& vertices, const Color& color, bool outline = true);

        /** @brief Draws an ellipse outline. */
        void DrawEllipse(const Vector2& center, const Vector2& radius, int sides, const Color& color);

        /**
         * @brief Draws a solid (filled) ellipse with an optional outline.
         * @param outline When true, an outline is drawn over the filled ellipse.
         */
        void DrawSolidEllipse(const Vector2& center, const Vector2& radius, int sides, const Color& color, bool outline = true);

    private:
        [[nodiscard]] static std::vector<Vector2> CreateEllipse(float rx, float ry, int sides);

        PrimitiveBatch* primitiveBatch_;
    };
}
