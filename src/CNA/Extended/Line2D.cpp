// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Line2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "CNA/Extended/Ray2D.hpp"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace CNA::Extended
{
    Line2D::Line2D(const Vector2& normal, const float distance) : Distance(distance), Normal(normal)
    {
    }

    Line2D Line2D::CreateFromPointAndNormal(const Vector2& point, const Vector2& normal)
    {
        const Vector2 n = Vector2::Normalize(normal);
        const float d = Vector2::Dot(n, point);
        return Line2D(n, d);
    }

    Line2D Line2D::CreateFromTwoPoints(const Vector2& p1, const Vector2& p2)
    {
        const Vector2 direction = p2 - p1;
        const float lengthSquared = direction.LengthSquared();

        if (lengthSquared < Collision2D::Epsilon * Collision2D::Epsilon)
        {
            throw std::invalid_argument("Points must be distinct to define a line.");
        }

        // Normal is perpendicular to direction (rotate 90deg CCW)
        const Vector2 normal(-direction.Y, direction.X);
        return CreateFromPointAndNormal(p1, normal);
    }

    Line2D Line2D::CreateFromPointAndDirection(const Vector2& point, const Vector2& direction)
    {
        // Normal is perpendicular to direction (rotate 90deg CCW)
        const Vector2 normal = Vector2::Normalize(Vector2(-direction.Y, direction.X));
        const float distance = Vector2::Dot(normal, point);
        return Line2D(normal, distance);
    }

    float Line2D::DistanceToPoint(const Vector2& point) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.1 "Closest Point on Plane To Point"
        // Adapted from 3D plane to 2D line using implicit line equation
        return Vector2::Dot(Normal, point) - Distance;
    }

    Vector2 Line2D::ClosestPoint(const Vector2& point, float& distanceAlongLine) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 5.1.2 "Closest Point on Line Segment to Point"
        // Applied to an infinite line (no clamping of t), as described by Ericson.
        const Vector2 n = Normal;
        const float nn = Vector2::Dot(n, n);
        if (nn <= Collision2D::Epsilon)
        {
            // Degenerate line, normal has no meaningful direction. Treat line as a single point.
            distanceAlongLine = 0.0f;
            return Vector2::Zero;
        }

        // Compute a specific point 'a' on the line by scaling the normal.
        const Vector2 a = n * (Distance / nn);

        // Compute a direction vector 'ab' that lies along the line.
        const Vector2 ab(-n.Y, n.X);

        const float denom = Vector2::Dot(ab, ab);
        distanceAlongLine = Vector2::Dot(point - a, ab);
        distanceAlongLine /= denom;

        return a + distanceAlongLine * ab;
    }

    Line2D Line2D::Normalize(const Line2D& line)
    {
        Line2D result;
        Normalize(line, result);
        return result;
    }

    void Line2D::Normalize(const Line2D& value, Line2D& result)
    {
        const float length = value.Normal.Length();
        if (length < Collision2D::Epsilon)
        {
            result = value;
            return;
        }

        result = Line2D(value.Normal / length, value.Distance / length);
    }

    void Line2D::Normalize()
    {
        const float length = Normal.Length();
        if (length > Collision2D::Epsilon)
        {
            Normal = Normal / length;
            Distance = Distance / length;
        }
    }

    bool Line2D::Intersects(const Line2D& other, std::optional<Vector2>& point) const
    {
        // Use implicit line representation and Cramer's rule to solve a 2D line-line intersection
        const float cross = Normal.X * other.Normal.Y - Normal.Y * other.Normal.X;
        if (std::abs(cross) < Collision2D::Epsilon)
        {
            // Lines are parallel or coincident
            point = std::nullopt;
            return false;
        }

        const float x = (Distance * other.Normal.Y - other.Distance * Normal.Y) / cross;
        const float y = (other.Distance * Normal.X - Distance * other.Normal.X) / cross;
        point = Vector2(x, y);
        return true;
    }

    bool Line2D::Intersects(const Line2D& other) const
    {
        std::optional<Vector2> point;
        return Intersects(other, point);
    }

    bool Line2D::Intersects(const Ray2D& ray, std::optional<float>& distanceAlongRay, std::optional<Vector2>& point) const
    {
        float t = 0.0f;
        if (!Collision2D::SolveParametricIntersectionWithImplicitLine(Normal, Distance, ray.Origin, ray.Direction, t))
        {
            // Parallel or coincident
            distanceAlongRay = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Ray only intersects in forward direction
        if (t < 0.0f)
        {
            distanceAlongRay = std::nullopt;
            point = std::nullopt;
            return false;
        }

        distanceAlongRay = t;
        point = ray.Origin + t * ray.Direction;
        return true;
    }

    bool Line2D::Intersects(const Ray2D& ray) const
    {
        std::optional<float> distanceAlongRay;
        std::optional<Vector2> point;
        return Intersects(ray, distanceAlongRay, point);
    }

    bool Line2D::Intersects(const LineSegment2D& segment, std::optional<float>& distanceAlongSegment, std::optional<Vector2>& point) const
    {
        const Vector2 ab = segment.End - segment.Start;

        float t = 0.0f;
        if (!Collision2D::SolveParametricIntersectionWithImplicitLine(Normal, Distance, segment.Start, ab, t))
        {
            distanceAlongSegment = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Check if the intersection is within the segment bounds
        if (t < 0.0f || t > 1.0f)
        {
            distanceAlongSegment = std::nullopt;
            point = std::nullopt;
            return false;
        }

        distanceAlongSegment = t;
        point = segment.Start + t * ab;
        return true;
    }

    bool Line2D::Intersects(const LineSegment2D& segment) const
    {
        std::optional<float> distanceAlongSegment;
        std::optional<Vector2> point;
        return Intersects(segment, distanceAlongSegment, point);
    }

    bool Line2D::Intersects(const BoundingCircle2D& circle) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Distance-based intersection of implicit line with circle
        // Derived from Section 5.2.2 "Testing Sphere Against Plane" (2D reduction)
        const float signedDist = DistanceToPoint(circle.Center);

        // Line intersects circle if perpendicular distance is within radius
        return std::abs(signedDist) <= circle.Radius;
    }

    bool Line2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Intersection of an implicit line with a 2D capsule (line segment swept by a circle)
        // Derived from Section 4.5.1 "Sphere-swept Volume Intersection" and Section 5.2.2
        // "Testing Sphere Against Plane"
        const float signedDistA = DistanceToPoint(capsule.PointA);
        const float signedDistB = DistanceToPoint(capsule.PointB);

        const float distToA = std::abs(signedDistA);
        const float distToB = std::abs(signedDistB);

        // minimum distance starts as the closer of the two end points
        float minDistSq = std::min(distToA * distToA, distToB * distToB);

        // Check if capsule's medial segment is not degenerate
        const Vector2 segmentDir = capsule.PointB - capsule.PointA;
        const float segmentLenSq = segmentDir.LengthSquared();

        if (segmentLenSq > Collision2D::Epsilon * Collision2D::Epsilon)
        {
            // If endpoints are on opposite sides of the line, the segment crosses it
            if (signedDistA * signedDistB <= 0.0f)
            {
                minDistSq = 0.0f;
            }
        }

        // Line intersects capsule if minimum distance is within radius
        return minDistSq <= capsule.Radius * capsule.Radius;
    }

    bool Line2D::Intersects(const BoundingBox2D& box) const
    {
        const Vector2 n = Normal;
        const float nn = Vector2::Dot(n, n);

        // Check for degenerate line
        if (nn <= Collision2D::Epsilon * Collision2D::Epsilon)
            return false;

        // Point on the line: a = n * (d / Dot(n,n))
        const Vector2 origin = n * (Distance / nn);

        // Direction along the line (perpendicular to normal)
        const Vector2 dir(-n.Y, n.X);

        float tEnter = 0.0f;
        float tExit = 0.0f;
        return Collision2D::ClipLineToAabb(
            origin, dir, box.Min, box.Max, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), tEnter, tExit);
    }

    bool Line2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        const Vector2 n = Normal;
        const float nn = Vector2::Dot(n, n);

        // Handle degenerate line
        if (nn <= Collision2D::Epsilon * Collision2D::Epsilon)
            return false;

        // Get a world space point and direction for the line
        const Vector2 a = n * (Distance / nn);
        const Vector2 dir(-n.Y, n.X);

        // Transform line into OBB local space
        const Vector2 diff = a - obb.Center;
        const Vector2 localOrigin(Vector2::Dot(diff, obb.AxisX), Vector2::Dot(diff, obb.AxisY));
        const Vector2 localDirection(Vector2::Dot(dir, obb.AxisX), Vector2::Dot(dir, obb.AxisY));

        // Local OBB is just an AABB [-halfExtents, +halfExtents]
        float tEnter = 0.0f;
        float tExit = 0.0f;
        return Collision2D::ClipLineToAabb(localOrigin, localDirection, -obb.HalfExtents, obb.HalfExtents,
            std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), tEnter, tExit);
    }

    bool Line2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        const Vector2 n = Normal;
        const float nn = Vector2::Dot(n, n);

        // Check for degenerate line
        if (nn <= Collision2D::Epsilon * Collision2D::Epsilon)
            return false;

        // A point on the line: a = n * (d / Dot(n,n))
        const Vector2 a = n * (Distance / nn);

        // A direction along the line (perpendicular to n)
        const Vector2 dir(-n.Y, n.X);

        // Clip infinite line against polygon half-spaces
        float tEnter = 0.0f;
        float tExit = 0.0f;
        return Collision2D::ClipLineToConvexPolygon(a, dir, polygon.Vertices, polygon.Normals,
            std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), tEnter, tExit);
    }

    void Line2D::Deconstruct(Vector2& normal, float& distance) const
    {
        normal = Normal;
        distance = Distance;
    }

    bool Line2D::Equals(const Line2D& other) const
    {
        return Normal == other.Normal && std::abs(Distance - other.Distance) < Collision2D::Epsilon;
    }

    int Line2D::GetHashCode() const
    {
        return Normal.GetHashCode() ^ static_cast<int>(std::hash<float>{}(Distance));
    }

    std::string Line2D::ToString() const
    {
        return "{Normal:" + Normal.ToString() + " Distance:" + std::to_string(Distance) + "}";
    }
}
