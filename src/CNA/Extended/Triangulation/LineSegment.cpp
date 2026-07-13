// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Triangulation/LineSegment.hpp"

#include "Microsoft/Xna/Framework/MathHelper.hpp"

namespace CNA::Extended::Triangulation
{
    using Microsoft::Xna::Framework::MathHelper;

    LineSegment::LineSegment(const Vertex& a, const Vertex& b) : A(a), B(b)
    {
    }

    std::optional<float> LineSegment::IntersectsWithRay(const Vector2& origin, const Vector2& direction) const
    {
        const float largestDistance = MathHelper::Max(A.Position.X - origin.X, B.Position.X - origin.X) * 2.0f;
        const LineSegment raySegment(Vertex(origin, 0), Vertex(origin + (direction * largestDistance), 0));

        const std::optional<Vector2> intersection = FindIntersection(*this, raySegment);
        std::optional<float> value;

        if (intersection.has_value())
        {
            value = Vector2::Distance(origin, intersection.value());
        }

        return value;
    }

    std::optional<Vector2> LineSegment::FindIntersection(const LineSegment& a, const LineSegment& b)
    {
        const float x1 = a.A.Position.X;
        const float y1 = a.A.Position.Y;
        const float x2 = a.B.Position.X;
        const float y2 = a.B.Position.Y;
        const float x3 = b.A.Position.X;
        const float y3 = b.A.Position.Y;
        const float x4 = b.B.Position.X;
        const float y4 = b.B.Position.Y;

        const float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

        const float uaNum = (x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3);
        const float ubNum = (x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3);

        const float ua = uaNum / denom;
        const float ub = ubNum / denom;

        if (MathHelper::Clamp(ua, 0.0f, 1.0f) != ua || MathHelper::Clamp(ub, 0.0f, 1.0f) != ub)
        {
            return std::nullopt;
        }

        return a.A.Position + (a.B.Position - a.A.Position) * ua;
    }
}
