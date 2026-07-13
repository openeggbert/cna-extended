// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/LineSegment2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"
#include "System/HashCode.hpp"

#include <cmath>

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

    float LineSegment2D::DistanceSquaredToPoint(const Vector2& point) const
    {
        float t = 0.0f;
        Vector2 closestPoint;
        return Collision2D::DistanceSquaredPointSegment(point, Start, End, t, closestPoint);
    }

    float LineSegment2D::DistanceToPoint(const Vector2& point) const
    {
        return std::sqrt(DistanceSquaredToPoint(point));
    }

    float LineSegment2D::DistanceSquaredToSegment(const LineSegment2D& other, float& distanceAlongSegment1, float& distanceAlongSegment2,
        Vector2& closestPoint1, Vector2& closestPoint2) const
    {
        return Collision2D::DistanceSquaredSegmentSegment(
            Start, End, other.Start, other.End, distanceAlongSegment1, distanceAlongSegment2, closestPoint1, closestPoint2);
    }

    float LineSegment2D::DistanceToSegment(const LineSegment2D& other) const
    {
        float distanceAlongSegment1 = 0.0f;
        float distanceAlongSegment2 = 0.0f;
        Vector2 closestPoint1;
        Vector2 closestPoint2;
        return std::sqrt(DistanceSquaredToSegment(other, distanceAlongSegment1, distanceAlongSegment2, closestPoint1, closestPoint2));
    }

    bool LineSegment2D::Intersects(const Line2D& line, std::optional<float>& distanceAlongSegment, std::optional<Vector2>& point) const
    {
        return line.Intersects(*this, distanceAlongSegment, point);
    }

    bool LineSegment2D::Intersects(const Line2D& line) const
    {
        return line.Intersects(*this);
    }

    bool LineSegment2D::Intersects(const Ray2D& ray, std::optional<float>& distanceAlongSegment, std::optional<float>& distanceAlongRay,
        std::optional<Vector2>& point) const
    {
        return ray.Intersects(*this, distanceAlongRay, distanceAlongSegment, point);
    }

    bool LineSegment2D::Intersects(const Ray2D& ray) const
    {
        return ray.Intersects(*this);
    }

    bool LineSegment2D::Intersects(const LineSegment2D& other, std::optional<float>& distanceAlongSegment1,
        std::optional<float>& distanceAlongSegment2, std::optional<Vector2>& point) const
    {
        float t1 = 0.0f;
        float t2 = 0.0f;
        if (!Collision2D::SolveParametricIntersection2D(Start, getDirectionProperty(), other.Start, other.getDirectionProperty(), t1, t2))
        {
            distanceAlongSegment1 = std::nullopt;
            distanceAlongSegment2 = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Clamp to segment bounds [0,1]
        if (t1 < 0.0f || t1 > 1.0f || t2 < 0.0f || t2 > 1.0f)
        {
            distanceAlongSegment1 = std::nullopt;
            distanceAlongSegment2 = std::nullopt;
            point = std::nullopt;
            return false;
        }

        distanceAlongSegment1 = t1;
        distanceAlongSegment2 = t2;
        point = Start + t1 * getDirectionProperty();
        return true;
    }

    bool LineSegment2D::Intersects(const LineSegment2D& other) const
    {
        std::optional<float> distanceAlongSegment1;
        std::optional<float> distanceAlongSegment2;
        std::optional<Vector2> point;
        return Intersects(other, distanceAlongSegment1, distanceAlongSegment2, point);
    }

    bool LineSegment2D::Intersects(const BoundingBox2D& box, std::optional<float>& tMin, std::optional<float>& tMax) const
    {
        const Vector2 d = getDirectionProperty();
        const float dd = Vector2::Dot(d, d);

        // Handle degenerate segment (zero length)
        if (dd < Collision2D::Epsilon * Collision2D::Epsilon)
        {
            const bool inside = Start.X >= box.Min.X && Start.X <= box.Max.X && Start.Y >= box.Min.Y && Start.Y <= box.Max.Y;

            if (inside)
            {
                tMin = 0.0f;
                tMax = 0.0f;
                return true;
            }

            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        float tEnter = 0.0f;
        float tExit = 0.0f;
        if (!Collision2D::ClipLineToAabb(Start, d, box.Min, box.Max, 0.0f, 1.0f, tEnter, tExit))
        {
            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        tMin = tEnter;
        tMax = tExit;
        return true;
    }

    bool LineSegment2D::Intersects(const BoundingBox2D& box) const
    {
        std::optional<float> tMin;
        std::optional<float> tMax;
        return Intersects(box, tMin, tMax);
    }

    bool LineSegment2D::Intersects(const BoundingCircle2D& circle, std::optional<float>& tSegmentMin, std::optional<float>& tSegmentMax) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of a segment with a circle (2D reduction)
        // Derived from Section 5.3.2 "Intersecting Ray or Segment Against Sphere"
        const Vector2 d = getDirectionProperty();
        const float dd = Vector2::Dot(d, d);

        // Handle degenerate segment (zero length)
        if (dd <= Collision2D::Epsilon * Collision2D::Epsilon)
        {
            const float distSq = Vector2::DistanceSquared(Start, circle.Center);
            if (distSq <= circle.Radius * circle.Radius)
            {
                tSegmentMin = 0.0f;
                tSegmentMax = 0.0f;
                return true;
            }

            tSegmentMin = std::nullopt;
            tSegmentMax = std::nullopt;
            return false;
        }

        float tMin = 0.0f;
        float tMax = 0.0f;
        if (!Collision2D::RayCircleIntersectionInterval(Start, d, circle.Center, circle.Radius, tMin, tMax))
        {
            tSegmentMin = std::nullopt;
            tSegmentMax = std::nullopt;
            return false;
        }

        // Clip ray interval to segment interval [0,1]
        float enter = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipInterval(tMin, tMax, 0.0f, 1.0f, enter, exit))
        {
            tSegmentMin = std::nullopt;
            tSegmentMax = std::nullopt;
            return false;
        }

        tSegmentMin = enter;
        tSegmentMax = exit;
        return true;
    }

    bool LineSegment2D::Intersects(const BoundingCircle2D& circle) const
    {
        std::optional<float> tSegmentMin;
        std::optional<float> tSegmentMax;
        return Intersects(circle, tSegmentMin, tSegmentMax);
    }

    bool LineSegment2D::Intersects(const BoundingCapsule2D& capsule, std::optional<float>& tMin, std::optional<float>& tMax) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of a segment with a capsule
        // Derived from Section 5.1.9 "Closest Points of Two Line Segments"
        // and Section 5.3.7 "Intersecting Ray or Segment Against Cylinder"
        const Vector2 d = getDirectionProperty();
        const float dd = Vector2::Dot(d, d);
        const float radiusSq = capsule.Radius * capsule.Radius;

        // Check for degenerate segment
        if (dd <= Collision2D::Epsilon * Collision2D::Epsilon)
        {
            // Segment is degenerate, treat as point
            float t = 0.0f;
            Vector2 closestPoint;
            const float distSq = Collision2D::DistanceSquaredPointSegment(Start, capsule.PointA, capsule.PointB, t, closestPoint);
            if (distSq <= radiusSq)
            {
                tMin = 0.0f;
                tMax = 0.0f;
                return true;
            }

            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        // Get intersection interval against the infinite parametric line P(t)=Start + t * d
        float t0 = 0.0f;
        float t1 = 0.0f;
        if (!Collision2D::RayCapsuleIntersectionInterval(Start, d, capsule.PointA, capsule.PointB, capsule.Radius, t0, t1))
        {
            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        float enter = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipInterval(t0, t1, 0.0f, 1.0f, enter, exit))
        {
            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        tMin = enter;
        tMax = exit;
        return true;
    }

    bool LineSegment2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        std::optional<float> tMin;
        std::optional<float> tMax;
        return Intersects(capsule, tMin, tMax);
    }

    bool LineSegment2D::Intersects(const OrientedBoundingBox2D& obb, std::optional<float>& tMin, std::optional<float>& tMax) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of a segment with an oriented box
        // Derived from Section 5.3.3 "Intersecting Ray or Segment Against Box"
        const Vector2 d = getDirectionProperty();
        const float dd = Vector2::Dot(d, d);
        const Vector2 diff = Start - obb.Center;

        // Check for degenerate segment (zero length)
        if (dd <= Collision2D::Epsilon * Collision2D::Epsilon)
        {
            // Segment is degenerate, treat it as a point
            const float x = Vector2::Dot(diff, obb.AxisX);
            const float y = Vector2::Dot(diff, obb.AxisY);

            const bool inside = std::abs(x) <= obb.HalfExtents.X && std::abs(y) <= obb.HalfExtents.Y;

            if (inside)
            {
                tMin = 0.0f;
                tMax = 0.0f;
                return true;
            }

            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        // Transform segment start and direction into OBB local space
        const Vector2 localOrigin(Vector2::Dot(diff, obb.AxisX), Vector2::Dot(diff, obb.AxisY));
        const Vector2 localDirection(Vector2::Dot(d, obb.AxisX), Vector2::Dot(d, obb.AxisY));

        // Intersect the parametric line: P(T) = localOrigin + t * localDirection
        // with local AABB [-halfExtent, +halfExtent]
        float enter = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipLineToAabb(localOrigin, localDirection, -obb.HalfExtents, obb.HalfExtents, 0.0f, 1.0f, enter, exit))
        {
            tMin = std::nullopt;
            tMax = std::nullopt;
            return false;
        }

        tMin = enter;
        tMax = exit;
        return true;
    }

    bool LineSegment2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        std::optional<float> tMin;
        std::optional<float> tMax;
        return Intersects(obb, tMin, tMax);
    }

    bool LineSegment2D::Intersects(
        const BoundingPolygon2D& polygon, std::optional<float>& tMin, std::optional<float>& tMax, std::optional<Vector2>& point) const
    {
        const Vector2 d = getDirectionProperty();
        const float dd = Vector2::Dot(d, d);

        // Check for degenerate segment
        if (dd <= Collision2D::Epsilon * Collision2D::Epsilon)
        {
            // Segment is degenerate, treat as point
            if (polygon.Contains(Start) == ContainmentType::Contains)
            {
                tMin = 0.0f;
                tMax = 0.0f;
                point = Start;
                return true;
            }

            tMin = std::nullopt;
            tMax = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Clip the segment's parametric line against the polygon
        float t0 = 0.0f;
        float t1 = 0.0f;
        if (!Collision2D::ClipLineToConvexPolygon(Start, d, polygon.Vertices, polygon.Normals, 0.0f, 1.0f, t0, t1))
        {
            tMin = std::nullopt;
            tMax = std::nullopt;
            point = std::nullopt;
            return false;
        }

        tMin = t0;
        tMax = t1;
        point = Start + d * t0;
        return true;
    }

    bool LineSegment2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        std::optional<float> tMin;
        std::optional<float> tMax;
        std::optional<Vector2> point;
        return Intersects(polygon, tMin, tMax, point);
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
