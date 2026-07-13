// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/ShapeExtensions.hpp"

#include "Microsoft/Xna/Framework/Graphics/GraphicsDevice.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteEffects.hpp"
#include "Microsoft/Xna/Framework/Graphics/SurfaceFormat.hpp"
#include "Microsoft/Xna/Framework/Graphics/Texture2D.hpp"
#include "System/EventArgs.hpp"
#include "System/Object.hpp"

#include <cmath>
#include <memory>
#include <optional>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Graphics::SpriteEffects;
    using Microsoft::Xna::Framework::Graphics::SurfaceFormat;
    using Microsoft::Xna::Framework::Graphics::Texture2D;

    namespace
    {
        // Upstream keeps this cache as a `private static` field shared across every SpriteBatch
        // this static class is ever used with (not one cache per SpriteBatch instance) -- a
        // translation-unit-local static reproduces that shared-across-calls lifetime.
        std::unique_ptr<Texture2D> whitePixelTexture;

        Texture2D& GetTexture(SpriteBatch& spriteBatch)
        {
            if (!whitePixelTexture || whitePixelTexture->getIsDisposedProperty())
            {
                whitePixelTexture =
                    std::make_unique<Texture2D>(*spriteBatch.getGraphicsDeviceProperty(), 1, 1, false, SurfaceFormat::Color);
                const Color white = Color::White;
                whitePixelTexture->SetData(&white, 1);
                spriteBatch.Disposing += [](System::Object*, const System::EventArgs&) { whitePixelTexture.reset(); };
            }

            return *whitePixelTexture;
        }

        void DrawPolygonEdge(
            SpriteBatch& spriteBatch, Texture2D& texture, const Vector2& point1, const Vector2& point2, const Color& color,
            float thickness, float layerDepth)
        {
            const float length = Vector2::Distance(point1, point2);
            const float angle = std::atan2(point2.Y - point1.Y, point2.X - point1.X);
            const Vector2 scale(length, thickness);
            spriteBatch.Draw(texture, point1, std::nullopt, color, angle, Vector2::Zero, scale, SpriteEffects::None, layerDepth);
        }

        std::vector<Vector2> CreateArc(float radius, int sides, float startAngle, float sweepAngle)
        {
            std::vector<Vector2> points(static_cast<std::size_t>(sides) + 1);
            const float step = sweepAngle / static_cast<float>(sides);
            float theta = startAngle;

            for (int i = 0; i <= sides; ++i, theta += step)
            {
                points[static_cast<std::size_t>(i)] = Vector2(radius * std::cos(theta), radius * std::sin(theta));
            }

            return points;
        }

        std::vector<Vector2> CreateCircle(double radius, int sides)
        {
            constexpr double max = 2.0 * 3.14159265358979323846;
            std::vector<Vector2> points(static_cast<std::size_t>(sides));
            const double step = max / static_cast<double>(sides);
            double theta = 0.0;

            for (int i = 0; i < sides; ++i)
            {
                points[static_cast<std::size_t>(i)] =
                    Vector2(static_cast<float>(radius * std::cos(theta)), static_cast<float>(radius * std::sin(theta)));
                theta += step;
            }

            return points;
        }

        std::vector<Vector2> CreateEllipse(float rx, float ry, int sides)
        {
            std::vector<Vector2> vertices(static_cast<std::size_t>(sides));

            double t = 0.0;
            const double dt = 2.0 * 3.14159265358979323846 / static_cast<double>(sides);
            for (int i = 0; i < sides; ++i, t += dt)
            {
                const auto x = static_cast<float>(rx * std::cos(t));
                const auto y = static_cast<float>(ry * std::sin(t));
                vertices[static_cast<std::size_t>(i)] = Vector2(x, y);
            }
            return vertices;
        }
    }

    void DrawPolygon(SpriteBatch& spriteBatch, const Vector2& position, const Shapes::Polygon& polygon, const Color& color,
        float thickness, float layerDepth)
    {
        DrawPolygon(spriteBatch, position, polygon.getVerticesProperty(), color, thickness, layerDepth);
    }

    void DrawPolygon(SpriteBatch& spriteBatch, const Vector2& offset, const std::vector<Vector2>& points, const Color& color,
        float thickness, float layerDepth)
    {
        if (points.empty())
        {
            return;
        }

        if (points.size() == 1)
        {
            DrawPoint(spriteBatch, points[0], color, static_cast<float>(static_cast<int>(thickness)));
            return;
        }

        Texture2D& texture = GetTexture(spriteBatch);

        for (std::size_t i = 0; i < points.size() - 1; ++i)
        {
            DrawPolygonEdge(spriteBatch, texture, points[i] + offset, points[i + 1] + offset, color, thickness, layerDepth);
        }

        DrawPolygonEdge(spriteBatch, texture, points[points.size() - 1] + offset, points[0] + offset, color, thickness, layerDepth);
    }

    void FillRectangle(SpriteBatch& spriteBatch, const RectangleF& rectangle, const Color& color, float layerDepth)
    {
        FillRectangle(spriteBatch, rectangle.getPositionProperty(), rectangle.getSizeProperty(), color, layerDepth);
    }

    void FillRectangle(SpriteBatch& spriteBatch, const Vector2& location, const SizeF& size, const Color& color, float layerDepth)
    {
        const Vector2 scale(size.Width, size.Height);
        spriteBatch.Draw(GetTexture(spriteBatch), location, std::nullopt, color, 0.0f, Vector2::Zero, scale, SpriteEffects::None,
            layerDepth);
    }

    void FillRectangle(SpriteBatch& spriteBatch, float x, float y, float width, float height, const Color& color, float layerDepth)
    {
        FillRectangle(spriteBatch, Vector2(x, y), SizeF(width, height), color, layerDepth);
    }

    void DrawRectangle(SpriteBatch& spriteBatch, const RectangleF& rectangle, const Color& color, float thickness, float layerDepth)
    {
        Texture2D& texture = GetTexture(spriteBatch);
        const Vector2 topLeft(rectangle.X, rectangle.Y);
        const Vector2 topRight(rectangle.getRightProperty() - thickness, rectangle.Y);
        const Vector2 bottomLeft(rectangle.X, rectangle.getBottomProperty() - thickness);
        const Vector2 horizontalScale(rectangle.Width, thickness);
        const Vector2 verticalScale(thickness, rectangle.Height);

        spriteBatch.Draw(
            texture, topLeft, std::nullopt, color, 0.0f, Vector2::Zero, horizontalScale, SpriteEffects::None, layerDepth);
        spriteBatch.Draw(texture, topLeft, std::nullopt, color, 0.0f, Vector2::Zero, verticalScale, SpriteEffects::None, layerDepth);
        spriteBatch.Draw(
            texture, topRight, std::nullopt, color, 0.0f, Vector2::Zero, verticalScale, SpriteEffects::None, layerDepth);
        spriteBatch.Draw(
            texture, bottomLeft, std::nullopt, color, 0.0f, Vector2::Zero, horizontalScale, SpriteEffects::None, layerDepth);
    }

    void DrawRectangle(
        SpriteBatch& spriteBatch, const Vector2& location, const SizeF& size, const Color& color, float thickness, float layerDepth)
    {
        DrawRectangle(spriteBatch, RectangleF(location.X, location.Y, size.Width, size.Height), color, thickness, layerDepth);
    }

    void DrawRectangle(
        SpriteBatch& spriteBatch, float x, float y, float width, float height, const Color& color, float thickness, float layerDepth)
    {
        DrawRectangle(spriteBatch, RectangleF(x, y, width, height), color, thickness, layerDepth);
    }

    void DrawLine(
        SpriteBatch& spriteBatch, float x1, float y1, float x2, float y2, const Color& color, float thickness, float layerDepth)
    {
        DrawLine(spriteBatch, Vector2(x1, y1), Vector2(x2, y2), color, thickness, layerDepth);
    }

    void DrawLine(
        SpriteBatch& spriteBatch, const Vector2& point1, const Vector2& point2, const Color& color, float thickness, float layerDepth)
    {
        const float distance = Vector2::Distance(point1, point2);
        const float angle = std::atan2(point2.Y - point1.Y, point2.X - point1.X);

        DrawLine(spriteBatch, point1, distance, angle, color, thickness, layerDepth);
    }

    void DrawLine(
        SpriteBatch& spriteBatch, const Vector2& point, float length, float angle, const Color& color, float thickness,
        float layerDepth)
    {
        const Vector2 origin(0.0f, 0.5f);
        const Vector2 scale(length, thickness);
        spriteBatch.Draw(GetTexture(spriteBatch), point, std::nullopt, color, angle, origin, scale, SpriteEffects::None, layerDepth);
    }

    void DrawPoint(SpriteBatch& spriteBatch, float x, float y, const Color& color, float size, float layerDepth)
    {
        DrawPoint(spriteBatch, Vector2(x, y), color, size, layerDepth);
    }

    void DrawPoint(SpriteBatch& spriteBatch, const Vector2& position, const Color& color, float size, float layerDepth)
    {
        const Vector2 scale = Vector2::One * size;
        const Vector2 offset = Vector2(0.5f, 0.5f) - Vector2(size * 0.5f, size * 0.5f);
        spriteBatch.Draw(
            GetTexture(spriteBatch), position + offset, std::nullopt, color, 0.0f, Vector2::Zero, scale, SpriteEffects::None,
            layerDepth);
    }

    void DrawCircle(SpriteBatch& spriteBatch, const CircleF& circle, int sides, const Color& color, float thickness, float layerDepth)
    {
        DrawCircle(spriteBatch, circle.Center, circle.Radius, sides, color, thickness, layerDepth);
    }

    void DrawCircle(
        SpriteBatch& spriteBatch, const Vector2& center, float radius, int sides, const Color& color, float thickness,
        float layerDepth)
    {
        DrawPolygon(spriteBatch, center, CreateCircle(radius, sides), color, thickness, layerDepth);
    }

    void DrawCircle(
        SpriteBatch& spriteBatch, float x, float y, float radius, int sides, const Color& color, float thickness, float layerDepth)
    {
        DrawPolygon(spriteBatch, Vector2(x, y), CreateCircle(radius, sides), color, thickness, layerDepth);
    }

    void DrawEllipse(
        SpriteBatch& spriteBatch, const Vector2& center, const Vector2& radius, int sides, const Color& color, float thickness,
        float layerDepth)
    {
        DrawPolygon(spriteBatch, center, CreateEllipse(radius.X, radius.Y, sides), color, thickness, layerDepth);
    }

    void DrawArc(
        SpriteBatch& spriteBatch, const Vector2& center, float radius, float startAngle, float sweepAngle, int sides,
        const Color& color, float thickness, float layerDepth)
    {
        Texture2D& texture = GetTexture(spriteBatch);
        const std::vector<Vector2> arc = CreateArc(radius, sides, startAngle, sweepAngle);

        for (std::size_t i = 0; i + 1 < arc.size(); ++i)
        {
            DrawPolygonEdge(spriteBatch, texture, center + arc[i], center + arc[i + 1], color, thickness, layerDepth);
        }
    }

    void DrawArc(
        SpriteBatch& spriteBatch, float x, float y, float radius, float startAngle, float sweepAngle, int sides, const Color& color,
        float thickness, float layerDepth)
    {
        DrawArc(spriteBatch, Vector2(x, y), radius, startAngle, sweepAngle, sides, color, thickness, layerDepth);
    }
}
