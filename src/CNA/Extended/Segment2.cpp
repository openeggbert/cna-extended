// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Segment2.hpp"

#include "CNA/Extended/BoundingRectangle.hpp"
#include "CNA/Extended/PrimitivesHelper.hpp"
#include "CNA/Extended/RectangleF.hpp"

#include <cmath>
#include <functional>
#include <limits>

namespace CNA::Extended
{
    Segment2::Segment2(const Vector2& start, const Vector2& end) : Start(start), End(end)
    {
    }

    Segment2::Segment2(const float x1, const float y1, const float x2, const float y2)
        : Segment2(Vector2(x1, y1), Vector2(x2, y2))
    {
    }

    Vector2 Segment2::ClosestPointTo(const Vector2& point) const
    {
        // Computes the parameterized position: d(t) = Start + t * (End - Start)
        Vector2 startToEnd = End - Start;
        const Vector2 startToPoint = point - Start;

        // Project arbitrary point onto the line segment, deferring the division.
        float t = Vector2::Dot(startToEnd, startToPoint);
        // If outside segment, clamp t (and therefore d) to the closest endpoint.
        if (t <= 0.0f)
        {
            return Start;
        }

        // Always nonnegative since denom = (||vector||)^2.
        const float denominator = Vector2::Dot(startToEnd, startToEnd);
        if (t >= denominator)
        {
            return End;
        }

        // The point projects inside the [Start, End] interval, must do deferred division now.
        t /= denominator;
        startToEnd = startToEnd * t;
        return Vector2(Start.X + startToEnd.X, Start.Y + startToEnd.Y);
    }

    float Segment2::SquaredDistanceTo(const Vector2& point) const
    {
        // See this file's header comment: this reproduces a likely upstream bug on purpose. The
        // `dot >= startToEndDistanceSquared` branch computes `endToPoint.Dot(endToPoint)` but
        // upstream never returns it, so execution always falls through to the final formula.
        const Vector2 startToEnd = End - Start;
        const Vector2 startToPoint = point - Start;
        const Vector2 endToPoint = point - End;

        // Handle cases where the point projects outside the line segment.
        const float dot = Vector2::Dot(startToPoint, startToEnd);
        const float startToPointDistanceSquared = Vector2::Dot(startToPoint, startToPoint);
        if (dot <= 0.0f)
        {
            return startToPointDistanceSquared;
        }
        const float startToEndDistanceSquared = Vector2::Dot(startToEnd, startToEnd);
        if (dot >= startToEndDistanceSquared)
        {
            // Upstream computes this and discards it -- preserved exactly, see header comment.
            [[maybe_unused]] const float discarded = Vector2::Dot(endToPoint, endToPoint);
        }
        // Handle the case where the point projects onto the line segment.
        return startToPointDistanceSquared - dot * dot / startToEndDistanceSquared;
    }

    float Segment2::DistanceTo(const Vector2& point) const
    {
        return std::sqrt(SquaredDistanceTo(point));
    }

    bool Segment2::Intersects(const RectangleF& rectangle, Vector2& intersectionPoint) const
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 5.3; Basic Primitive
        // Tests - Intersecting Lines, Rays, and (Directed Segments). pg 179-181
        const Vector2 minimumPoint = rectangle.getTopLeftProperty();
        const Vector2 maximumPoint = rectangle.getBottomRightProperty();
        float minimumDistance = std::numeric_limits<float>::lowest();
        float maximumDistance = std::numeric_limits<float>::max();

        const Vector2 direction = End - Start;
        if (!PrimitivesHelper::IntersectsSlab(Start.X, direction.X, minimumPoint.X, maximumPoint.X, minimumDistance, maximumDistance))
        {
            intersectionPoint = Vector2(std::numeric_limits<float>::quiet_NaN());
            return false;
        }

        if (!PrimitivesHelper::IntersectsSlab(Start.Y, direction.Y, minimumPoint.Y, maximumPoint.Y, minimumDistance, maximumDistance))
        {
            intersectionPoint = Vector2(std::numeric_limits<float>::quiet_NaN());
            return false;
        }

        // Segment intersects the 2 slabs.
        if (minimumDistance <= 0)
        {
            intersectionPoint = Start;
        }
        else
        {
            intersectionPoint = minimumDistance * direction;
            intersectionPoint.X += Start.X;
            intersectionPoint.Y += Start.Y;
        }

        return true;
    }

    bool Segment2::Intersects(const BoundingRectangle& boundingRectangle, Vector2& intersectionPoint) const
    {
        // Real-Time Collision Detection, Christer Ericson, 2005. Chapter 5.3; Basic Primitive
        // Tests - Intersecting Lines, Rays, and (Directed Segments). pg 179-181
        const Vector2 minimumPoint = boundingRectangle.Center - boundingRectangle.HalfExtents;
        const Vector2 maximumPoint = boundingRectangle.Center + boundingRectangle.HalfExtents;
        float minimumDistance = std::numeric_limits<float>::lowest();
        float maximumDistance = std::numeric_limits<float>::max();

        const Vector2 direction = End - Start;
        if (!PrimitivesHelper::IntersectsSlab(Start.X, direction.X, minimumPoint.X, maximumPoint.X, minimumDistance, maximumDistance))
        {
            intersectionPoint = Vector2(std::numeric_limits<float>::quiet_NaN());
            return false;
        }

        if (!PrimitivesHelper::IntersectsSlab(Start.Y, direction.Y, minimumPoint.Y, maximumPoint.Y, minimumDistance, maximumDistance))
        {
            intersectionPoint = Vector2(std::numeric_limits<float>::quiet_NaN());
            return false;
        }

        // Segment intersects the 2 slabs.
        if (minimumDistance <= 0)
        {
            intersectionPoint = Start;
        }
        else
        {
            intersectionPoint = minimumDistance * direction;
            intersectionPoint.X += Start.X;
            intersectionPoint.Y += Start.Y;
        }

        return true;
    }

    bool Segment2::Equals(const Segment2& segment) const
    {
        return Start == segment.Start && End == segment.End;
    }

    int Segment2::GetHashCode() const
    {
        // Matches upstream's `unchecked { (Start.GetHashCode() * 397) ^ End.GetHashCode(); }`.
        return (Start.GetHashCode() * 397) ^ End.GetHashCode();
    }

    std::string Segment2::ToString() const
    {
        return Start.ToString() + " -> " + End.ToString();
    }
}
