// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Math/ShapeExtensions.cs (upstream static class
// `ShapeExtensions`). Despite living under the `Math/` folder upstream, its declared namespace is
// the C# root `MonoGame.Extended` -- matching this project's "namespace follows the actual C#
// declaration, not the file's folder" convention (see `Shapes::Polygon`/`Polyline`'s own header
// comments for the same precedent), this ports into the root `CNA::Extended` namespace, not
// `CNA::Extended::Graphics`, even though every method here is graphics-related. Extension methods
// -> free functions, matching this project's established convention. `IReadOnlyList<Vector2>` has
// no direct C++ equivalent; translated as `const std::vector<Vector2>&`, matching
// `Polygon::getVerticesProperty()`'s own return type so `DrawPolygon(SpriteBatch&, Vector2,
// const Polygon&, ...)` can forward its vertices directly.
//
// These methods are intended for prototyping/debug visualization only (per upstream's own doc
// remarks): outline joints overlap (visible double-blending with semi-transparent colors), and
// filled *shapes* other than plain rectangles are not supported here -- see
// `CNA::Extended::VectorDraw::PrimitiveDrawing`/`PrimitiveBatch` (already ported, Phase 3) for
// filled-shape drawing with correct alpha blending.
#pragma once

#include "CNA/Extended/CircleF.hpp"
#include "CNA/Extended/RectangleF.hpp"
#include "CNA/Extended/Shapes/Polygon.hpp"
#include "CNA/Extended/Size.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <vector>

namespace Microsoft::Xna::Framework::Graphics
{
    class SpriteBatch;
}

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Color;
    using Microsoft::Xna::Framework::Vector2;
    using Microsoft::Xna::Framework::Graphics::SpriteBatch;

    /** @brief Draws a closed polygon outline from a Polygon shape. */
    void DrawPolygon(SpriteBatch& spriteBatch, const Vector2& position, const Shapes::Polygon& polygon, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws a closed polygon outline connecting @p points, each offset by @p offset. */
    void DrawPolygon(SpriteBatch& spriteBatch, const Vector2& offset, const std::vector<Vector2>& points, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws a filled rectangle. */
    void FillRectangle(SpriteBatch& spriteBatch, const RectangleF& rectangle, const Color& color, float layerDepth = 0.0f);
    /** @brief Draws a filled rectangle at @p location with the given @p size. */
    void FillRectangle(SpriteBatch& spriteBatch, const Vector2& location, const SizeF& size, const Color& color, float layerDepth = 0.0f);
    /** @brief Draws a filled rectangle. */
    void FillRectangle(
        SpriteBatch& spriteBatch, float x, float y, float width, float height, const Color& color, float layerDepth = 0.0f);

    /** @brief Draws a rectangle outline with the given line @p thickness. */
    void DrawRectangle(
        SpriteBatch& spriteBatch, const RectangleF& rectangle, const Color& color, float thickness = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws a rectangle outline at @p location with the given @p size. */
    void DrawRectangle(SpriteBatch& spriteBatch, const Vector2& location, const SizeF& size, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws a rectangle outline. */
    void DrawRectangle(SpriteBatch& spriteBatch, float x, float y, float width, float height, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws a line from (x1, y1) to (x2, y2). */
    void DrawLine(SpriteBatch& spriteBatch, float x1, float y1, float x2, float y2, const Color& color, float thickness = 1.0f,
        float layerDepth = 0.0f);
    /** @brief Draws a line from @p point1 to @p point2. */
    void DrawLine(SpriteBatch& spriteBatch, const Vector2& point1, const Vector2& point2, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws a line of @p length starting at @p point, rotated by @p angle radians. */
    void DrawLine(SpriteBatch& spriteBatch, const Vector2& point, float length, float angle, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws a point at (x, y). The center of the point is at the position. */
    void DrawPoint(SpriteBatch& spriteBatch, float x, float y, const Color& color, float size = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws a point at @p position. The center of the point is at the position. */
    void DrawPoint(SpriteBatch& spriteBatch, const Vector2& position, const Color& color, float size = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws a circle outline from a CircleF shape. */
    void DrawCircle(SpriteBatch& spriteBatch, const CircleF& circle, int sides, const Color& color, float thickness = 1.0f,
        float layerDepth = 0.0f);
    /** @brief Draws a circle outline centered at @p center. */
    void DrawCircle(SpriteBatch& spriteBatch, const Vector2& center, float radius, int sides, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws a circle outline centered at (x, y). */
    void DrawCircle(SpriteBatch& spriteBatch, float x, float y, float radius, int sides, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /** @brief Draws an ellipse outline centered at @p center. */
    void DrawEllipse(SpriteBatch& spriteBatch, const Vector2& center, const Vector2& radius, int sides, const Color& color,
        float thickness = 1.0f, float layerDepth = 0.0f);

    /**
     * @brief Draws an arc outline centered at @p center.
     * @param startAngle Starting angle in radians.
     * @param sweepAngle Sweep angle in radians; positive sweeps counter-clockwise. Use two pi for a full circle.
     * @param sides Number of line segments used to approximate the arc.
     */
    void DrawArc(SpriteBatch& spriteBatch, const Vector2& center, float radius, float startAngle, float sweepAngle, int sides,
        const Color& color, float thickness = 1.0f, float layerDepth = 0.0f);
    /** @brief Draws an arc outline centered at (x, y). See the Vector2-center overload for parameter details. */
    void DrawArc(SpriteBatch& spriteBatch, float x, float y, float radius, float startAngle, float sweepAngle, int sides,
        const Color& color, float thickness = 1.0f, float layerDepth = 0.0f);
}
