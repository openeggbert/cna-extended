// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingCapsule2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <numbers>

namespace CNA::Extended
{
    BoundingCapsule2D::BoundingCapsule2D(const Vector2& pointA, const Vector2& pointB, const float radius)
        : PointA(pointA), PointB(pointB), Radius(radius)
    {
    }

    float BoundingCapsule2D::getLengthProperty() const
    {
        return std::sqrt(getLengthSquaredProperty());
    }

    float BoundingCapsule2D::getLengthSquaredProperty() const
    {
        const Vector2 diff = PointB - PointA;
        return diff.X * diff.X + diff.Y * diff.Y;
    }

    Vector2 BoundingCapsule2D::getDirectionProperty() const
    {
        const Vector2 dir = PointB - PointA;
        const float lengthSquared = dir.X * dir.X + dir.Y * dir.Y;

        if (lengthSquared < Collision2D::Epsilon * Collision2D::Epsilon)
        {
            return Vector2::Zero;
        }

        return dir / std::sqrt(lengthSquared);
    }

    float BoundingCapsule2D::getAreaProperty() const
    {
        const float length = getLengthProperty();
        return length * (2.0f * Radius) + std::numbers::pi_v<float> * Radius * Radius;
    }

    BoundingCapsule2D BoundingCapsule2D::CreateFromCenterAndDirection(
        const Vector2& center, const Vector2& direction, const float length, const float radius)
    {
        // Check if the direction needs to be normalized and normalize it.
        const float lengthSq = direction.LengthSquared();
        Vector2 normalizedDir = Vector2::Zero;
        if (lengthSq >= Collision2D::Epsilon * Collision2D::Epsilon)
        {
            normalizedDir = direction / std::sqrt(lengthSq);
        }

        const Vector2 halfExtent = normalizedDir * (length * 0.5f);

        return BoundingCapsule2D(center - halfExtent, center + halfExtent, radius);
    }

    BoundingCapsule2D BoundingCapsule2D::CreateFromSegment(const LineSegment2D& segment, const float radius)
    {
        return BoundingCapsule2D(segment.Start, segment.End, radius);
    }

    BoundingCapsule2D BoundingCapsule2D::CreateMerged(const BoundingCapsule2D& original, const BoundingCapsule2D& additional)
    {
        const std::array<Vector2, 4> points = {original.PointA, original.PointB, additional.PointA, additional.PointB};
        const std::array<float, 4> radii = {original.Radius, original.Radius, additional.Radius, additional.Radius};

        // Find the pair of points that are farthest apart; those become the merged capsule's
        // endpoints (Ericson's "Sphere-Swept Volumes" / "Merging Two Spheres" approach).
        int bestI = 0;
        int bestJ = 1;
        float bestDistSq = Vector2::DistanceSquared(points[0], points[1]);
        for (int i = 0; i < 4; ++i)
        {
            for (int j = i + 1; j < 4; ++j)
            {
                const float distSq = Vector2::DistanceSquared(points[i], points[j]);
                if (distSq > bestDistSq)
                {
                    bestDistSq = distSq;
                    bestI = i;
                    bestJ = j;
                }
            }
        }

        const Vector2 newPointA = points[bestI];
        const Vector2 newPointB = points[bestJ];

        float newRadius = std::max(original.Radius, additional.Radius);
        const LineSegment2D newSegment(newPointA, newPointB);
        for (int i = 0; i < 4; ++i)
        {
            const float distToSegment = newSegment.DistanceToPoint(points[i]);
            const float requiredRadius = distToSegment + radii[i];
            newRadius = std::max(newRadius, requiredRadius);
        }

        return BoundingCapsule2D(newPointA, newPointB, newRadius);
    }

    BoundingCapsule2D BoundingCapsule2D::Transform(const Matrix& matrix) const
    {
        const Vector2 transformedA = Vector2::Transform(PointA, matrix);
        const Vector2 transformedB = Vector2::Transform(PointB, matrix);

        // Scale radius by maximum scale component
        const float scaleX = std::sqrt(matrix.M11 * matrix.M11 + matrix.M12 * matrix.M12);
        const float scaleY = std::sqrt(matrix.M21 * matrix.M21 + matrix.M22 * matrix.M22);
        const float transformedRadius = Radius * std::max(scaleX, scaleY);

        return BoundingCapsule2D(transformedA, transformedB, transformedRadius);
    }

    BoundingCapsule2D BoundingCapsule2D::Translate(const Vector2& translation) const
    {
        return BoundingCapsule2D(PointA + translation, PointB + translation, Radius);
    }

    void BoundingCapsule2D::Deconstruct(Vector2& pointA, Vector2& pointB, float& radius) const
    {
        pointA = PointA;
        pointB = PointB;
        radius = Radius;
    }

    ContainmentType BoundingCapsule2D::Contains(const Vector2& point) const
    {
        const float rr = Radius * Radius;
        float t = 0.0f;
        Vector2 closestPoint;
        const float d2 = Collision2D::DistanceSquaredPointSegment(point, PointA, PointB, t, closestPoint);
        if (d2 <= rr)
        {
            return ContainmentType::Contains;
        }
        return ContainmentType::Disjoint;
    }

    ContainmentType BoundingCapsule2D::Contains(const BoundingBox2D& aabb) const
    {
        return Collision2D::ContainsCapsuleAabb(PointA, PointB, Radius, aabb.Min, aabb.Max);
    }

    ContainmentType BoundingCapsule2D::Contains(const BoundingCircle2D& circle) const
    {
        return Collision2D::ContainsCapsuleCircle(PointA, PointB, Radius, circle.Center, circle.Radius);
    }

    ContainmentType BoundingCapsule2D::Contains(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::ContainsCapsuleObb(PointA, PointB, Radius, obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents);
    }

    ContainmentType BoundingCapsule2D::Contains(const BoundingCapsule2D& other) const
    {
        return Collision2D::ContainsCapsuleCapsule(PointA, PointB, Radius, other.PointA, other.PointB, other.Radius);
    }

    ContainmentType BoundingCapsule2D::Contains(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::ContainsCapsuleConvexPolygon(PointA, PointB, Radius, polygon.Vertices, polygon.Normals);
    }

    bool BoundingCapsule2D::Intersects(const BoundingCapsule2D& other) const
    {
        return Collision2D::IntersectsCapsuleCapsule(PointA, PointB, Radius, other.PointA, other.PointB, other.Radius);
    }

    bool BoundingCapsule2D::Intersects(const BoundingCircle2D& circle) const
    {
        return Collision2D::IntersectsCircleCapsule(circle.Center, circle.Radius, PointA, PointB, Radius);
    }

    bool BoundingCapsule2D::Intersects(const BoundingBox2D& box) const
    {
        return Collision2D::IntersectsAabbCapsule(box.Min, box.Max, PointA, PointB, Radius);
    }

    bool BoundingCapsule2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::IntersectsObbCapsule(obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents, PointA, PointB, Radius);
    }

    bool BoundingCapsule2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::IntersectsCapsuleConvexPolygon(PointA, PointB, Radius, polygon.Vertices, polygon.Normals);
    }

    bool BoundingCapsule2D::TryGetCollision(const BoundingCircle2D& circle, CollisionResult2D& result) const
    {
        CollisionResult2D circleResult;
        if (!Collision2D::TryGetCollisionCircleCapsule(circle.Center, circle.Radius, PointA, PointB, Radius, circleResult))
        {
            result = CollisionResult2D::None;
            return false;
        }
        result = circleResult.Invert();
        return true;
    }

    bool BoundingCapsule2D::Equals(const BoundingCapsule2D& other) const
    {
        return PointA == other.PointA && PointB == other.PointB && Radius == other.Radius;
    }

    int BoundingCapsule2D::GetHashCode() const
    {
        return PointA.GetHashCode() ^ PointB.GetHashCode() ^ static_cast<int>(std::hash<float>{}(Radius));
    }

    std::string BoundingCapsule2D::ToString() const
    {
        return "{PointA:" + PointA.ToString() + " PointB:" + PointB.ToString() + " Radius:" + std::to_string(Radius) + "}";
    }
}
