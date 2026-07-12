// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/LineSegment2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "System/HashCode.hpp"

namespace CNA::Extended
{
    LineSegment2D::LineSegment2D(const Vector2& start, const Vector2& end) : Start(start), End(end)
    {
    }

    float LineSegment2D::getLengthProperty() const
    {
        return Vector2::Distance(Start, End);
    }

    float LineSegment2D::getLengthSquaredProperty() const
    {
        return Vector2::DistanceSquared(Start, End);
    }

    BoundingBox2D LineSegment2D::GetBounds() const
    {
        const Vector2 min = Vector2::Min(Start, End);
        const Vector2 max = Vector2::Max(Start, End);
        return BoundingBox2D(min, max);
    }

    Vector2 LineSegment2D::GetPoint(const float distanceAlongSegment) const
    {
        return Start + distanceAlongSegment * (End - Start);
    }

    Vector2 LineSegment2D::ClosestPoint(const Vector2& point, float& distanceAlongSegment) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.2 "Closest Point on Line Segment to Point"
        const Vector2 ab = End - Start;

        // Project point onto ab, but deferring divide by Dot(ab, ab)
        distanceAlongSegment = Vector2::Dot(point - Start, ab);
        if (distanceAlongSegment <= 0.0f)
        {
            // point projects outside the [a,b] interval, on the a side; clamp to a
            distanceAlongSegment = 0.0f;
            return Start;
        }

        const float denom = Vector2::Dot(ab, ab);
        if (distanceAlongSegment >= denom)
        {
            // point projects outside the [a,b] interval, on the b side; clamp to b
            distanceAlongSegment = 1.0f;
            return End;
        }

        // Point projects inside the [a,b] interval; must do deferred divide now
        distanceAlongSegment /= denom;
        return Start + distanceAlongSegment * ab;
    }

    void LineSegment2D::Deconstruct(Vector2& start, Vector2& end) const
    {
        start = Start;
        end = End;
    }

    bool LineSegment2D::Equals(const LineSegment2D& other) const
    {
        return Start == other.Start && End == other.End;
    }

    int LineSegment2D::GetHashCode() const
    {
        return System::HashCode::Combine(Start.GetHashCode(), End.GetHashCode());
    }

    std::string LineSegment2D::ToString() const
    {
        return "LineSegment2D { Start: " + Start.ToString() + ", End: " + End.ToString() +
               ", Length: " + std::to_string(getLengthProperty()) + " }";
    }
}
