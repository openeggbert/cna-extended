// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Line2D.hpp"

#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"

#include <cmath>
#include <functional>
#include <stdexcept>

namespace CNA::Extended
{
    namespace
    {
        // Duplicates MonoGame.Extended's Collision2D.Epsilon (Collision2D.cs:44), which is not yet
        // ported (scheduled for Phase 2). Point this at Collision2D::Epsilon once that lands.
        constexpr float kCollision2DEpsilonPending = 1e-6f;
    }

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

        if (lengthSquared < kCollision2DEpsilonPending * kCollision2DEpsilonPending)
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
        if (nn <= kCollision2DEpsilonPending)
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
        if (length < kCollision2DEpsilonPending)
        {
            result = value;
            return;
        }

        result = Line2D(value.Normal / length, value.Distance / length);
    }

    void Line2D::Normalize()
    {
        const float length = Normal.Length();
        if (length > kCollision2DEpsilonPending)
        {
            Normal = Normal / length;
            Distance = Distance / length;
        }
    }

    bool Line2D::Intersects(const Line2D& other, std::optional<Vector2>& point) const
    {
        // Use implicit line representation and Cramer's rule to solve a 2D line-line intersection
        const float cross = Normal.X * other.Normal.Y - Normal.Y * other.Normal.X;
        if (std::abs(cross) < kCollision2DEpsilonPending)
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

        if (segmentLenSq > kCollision2DEpsilonPending * kCollision2DEpsilonPending)
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

    void Line2D::Deconstruct(Vector2& normal, float& distance) const
    {
        normal = Normal;
        distance = Distance;
    }

    bool Line2D::Equals(const Line2D& other) const
    {
        return Normal == other.Normal && std::abs(Distance - other.Distance) < kCollision2DEpsilonPending;
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
