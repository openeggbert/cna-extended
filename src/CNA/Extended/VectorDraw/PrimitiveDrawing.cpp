// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/VectorDraw/PrimitiveDrawing.hpp"

#include "CNA/Extended/Triangulation/Triangulator.hpp"
#include "CNA/Extended/VectorDraw/PrimitiveBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/PrimitiveType.hpp"
#include "Microsoft/Xna/Framework/MathHelper.hpp"

#include <cmath>
#include <stdexcept>

namespace CNA::Extended::VectorDraw
{
    using CNA::Extended::Triangulation::Triangulator;
    using CNA::Extended::Triangulation::WindingOrder;
    using Microsoft::Xna::Framework::MathHelper;
    using Microsoft::Xna::Framework::Graphics::PrimitiveType;

    PrimitiveDrawing::PrimitiveDrawing(PrimitiveBatch& primitiveBatch) : primitiveBatch_(&primitiveBatch)
    {
    }

    void PrimitiveDrawing::DrawPoint(const Vector2& center, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        primitiveBatch_->AddVertex(center, color, PrimitiveType::LineList);
        primitiveBatch_->AddVertex(center, color, PrimitiveType::LineList);
    }

    void PrimitiveDrawing::DrawRectangle(const Vector2& location, float width, float height, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const std::vector<Vector2> rectVerts{Vector2(0, 0), Vector2(width, 0), Vector2(width, height), Vector2(0, height)};

        DrawPolygon(location, rectVerts, color);
    }

    void PrimitiveDrawing::DrawSolidRectangle(const Vector2& location, float width, float height, const Color& color, bool outline)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const std::vector<Vector2> rectVerts{Vector2(0, 0), Vector2(width, 0), Vector2(width, height), Vector2(0, height)};

        DrawSolidPolygon(location, rectVerts, color, outline);
    }

    void PrimitiveDrawing::DrawCircle(const Vector2& center, float radius, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const double increment = MathHelper::Pi * 2.0 / CircleSegments;
        double theta = 0.0;

        for (int i = 0; i < CircleSegments; i++)
        {
            const Vector2 v1 = center + radius * Vector2(static_cast<float>(std::cos(theta)), static_cast<float>(std::sin(theta)));
            const Vector2 v2 =
                center + radius * Vector2(static_cast<float>(std::cos(theta + increment)), static_cast<float>(std::sin(theta + increment)));

            primitiveBatch_->AddVertex(v1, color, PrimitiveType::LineList);
            primitiveBatch_->AddVertex(v2, color, PrimitiveType::LineList);

            theta += increment;
        }
    }

    void PrimitiveDrawing::DrawSolidCircle(const Vector2& center, float radius, const Color& color, bool outline)
    {
        DrawSolidCircle(center, radius, color, color, outline);
    }

    void PrimitiveDrawing::DrawSolidCircle(const Vector2& center, float radius, const Color& color, const Color& fillColor, bool outline)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const double increment = MathHelper::Pi * 2.0 / CircleSegments;
        double theta = 0.0;

        const Vector2 v0 = center + radius * Vector2(static_cast<float>(std::cos(theta)), static_cast<float>(std::sin(theta)));
        theta += increment;

        for (int i = 1; i < CircleSegments - 1; i++)
        {
            const Vector2 v1 = center + radius * Vector2(static_cast<float>(std::cos(theta)), static_cast<float>(std::sin(theta)));
            const Vector2 v2 =
                center + radius * Vector2(static_cast<float>(std::cos(theta + increment)), static_cast<float>(std::sin(theta + increment)));

            primitiveBatch_->AddVertex(v0, fillColor, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(v1, fillColor, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(v2, fillColor, PrimitiveType::TriangleList);

            theta += increment;
        }

        if (outline)
        {
            DrawCircle(center, radius, color);
        }
    }

    void PrimitiveDrawing::DrawArc(const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const float step = sweepAngle / static_cast<float>(sides);
        float theta = startAngle;

        for (int i = 0; i < sides; i++)
        {
            const Vector2 v1 = center + radius * Vector2(std::cos(theta), std::sin(theta));
            const Vector2 v2 = center + radius * Vector2(std::cos(theta + step), std::sin(theta + step));

            primitiveBatch_->AddVertex(v1, color, PrimitiveType::LineList);
            primitiveBatch_->AddVertex(v2, color, PrimitiveType::LineList);

            theta += step;
        }
    }

    void PrimitiveDrawing::DrawSolidArc(const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color)
    {
        DrawSolidArc(center, radius, startAngle, sweepAngle, sides, color, color);
    }

    void PrimitiveDrawing::DrawSolidArc(
        const Vector2& center, float radius, float startAngle, float sweepAngle, int sides, const Color& color, const Color& fillColor)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const float step = sweepAngle / static_cast<float>(sides);
        float theta = startAngle;

        for (int i = 0; i < sides; i++)
        {
            const Vector2 v1 = center + radius * Vector2(std::cos(theta), std::sin(theta));
            const Vector2 v2 = center + radius * Vector2(std::cos(theta + step), std::sin(theta + step));

            primitiveBatch_->AddVertex(center, fillColor, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(v1, fillColor, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(v2, fillColor, PrimitiveType::TriangleList);

            theta += step;
        }

        DrawArc(center, radius, startAngle, sweepAngle, sides, color);
    }

    void PrimitiveDrawing::DrawSegment(const Vector2& start, const Vector2& end, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        primitiveBatch_->AddVertex(start, color, PrimitiveType::LineList);
        primitiveBatch_->AddVertex(end, color, PrimitiveType::LineList);
    }

    void PrimitiveDrawing::DrawPolygon(const Vector2& position, const std::vector<Vector2>& vertices, const Color& color, bool closed)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const std::size_t count = vertices.size();

        for (std::size_t i = 0; i + 1 < count; i++)
        {
            primitiveBatch_->AddVertex(Vector2(vertices[i].X + position.X, vertices[i].Y + position.Y), color, PrimitiveType::LineList);
            primitiveBatch_->AddVertex(
                Vector2(vertices[i + 1].X + position.X, vertices[i + 1].Y + position.Y), color, PrimitiveType::LineList);
        }

        if (closed)
        {
            primitiveBatch_->AddVertex(
                Vector2(vertices[count - 1].X + position.X, vertices[count - 1].Y + position.Y), color, PrimitiveType::LineList);
            primitiveBatch_->AddVertex(Vector2(vertices[0].X + position.X, vertices[0].Y + position.Y), color, PrimitiveType::LineList);
        }
    }

    void PrimitiveDrawing::DrawSolidPolygon(const Vector2& position, const std::vector<Vector2>& vertices, const Color& color, bool outline)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        if (vertices.size() == 2)
        {
            DrawPolygon(position, vertices, color);
            return;
        }

        std::vector<Vector2> outVertices;
        std::vector<int> outIndices;
        Triangulator::Triangulate(vertices, WindingOrder::CounterClockwise, outVertices, outIndices);

        for (std::size_t i = 0; i + 2 < outIndices.size(); i += 3)
        {
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i])].X + position.X,
                    outVertices[static_cast<std::size_t>(outIndices[i])].Y + position.Y),
                color, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i + 1])].X + position.X,
                    outVertices[static_cast<std::size_t>(outIndices[i + 1])].Y + position.Y),
                color, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i + 2])].X + position.X,
                    outVertices[static_cast<std::size_t>(outIndices[i + 2])].Y + position.Y),
                color, PrimitiveType::TriangleList);
        }

        if (outline)
        {
            DrawPolygon(position, vertices, color);
        }
    }

    void PrimitiveDrawing::DrawEllipse(const Vector2& center, const Vector2& radius, int sides, const Color& color)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        DrawPolygon(center, CreateEllipse(radius.X, radius.Y, sides), color);
    }

    void PrimitiveDrawing::DrawSolidEllipse(const Vector2& center, const Vector2& radius, int sides, const Color& color, bool outline)
    {
        if (!primitiveBatch_->IsReady())
        {
            throw std::logic_error("BeginCustomDraw must be called before drawing anything.");
        }

        const std::vector<Vector2> vertices = CreateEllipse(radius.X, radius.Y, sides);

        std::vector<Vector2> outVertices;
        std::vector<int> outIndices;
        Triangulator::Triangulate(vertices, WindingOrder::CounterClockwise, outVertices, outIndices);

        for (std::size_t i = 0; i + 2 < outIndices.size(); i += 3)
        {
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i])].X + center.X,
                    outVertices[static_cast<std::size_t>(outIndices[i])].Y + center.Y),
                color, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i + 1])].X + center.X,
                    outVertices[static_cast<std::size_t>(outIndices[i + 1])].Y + center.Y),
                color, PrimitiveType::TriangleList);
            primitiveBatch_->AddVertex(
                Vector2(outVertices[static_cast<std::size_t>(outIndices[i + 2])].X + center.X,
                    outVertices[static_cast<std::size_t>(outIndices[i + 2])].Y + center.Y),
                color, PrimitiveType::TriangleList);
        }

        if (outline)
        {
            DrawPolygon(center, vertices, color);
        }
    }

    std::vector<Vector2> PrimitiveDrawing::CreateEllipse(float rx, float ry, int sides)
    {
        std::vector<Vector2> vertices(static_cast<std::size_t>(sides));
        double t = 0.0;
        const double dt = 2.0 * MathHelper::Pi / sides;

        for (int i = 0; i < sides; i++, t += dt)
        {
            vertices[static_cast<std::size_t>(i)] = Vector2(static_cast<float>(rx * std::cos(t)), static_cast<float>(ry * std::sin(t)));
        }

        return vertices;
    }
}
