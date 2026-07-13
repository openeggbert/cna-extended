// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingCircle2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include <cmath>
#include <functional>
#include <numbers>
#include <stdexcept>

namespace CNA::Extended
{
    namespace
    {
        // Duplicates MonoGame.Extended's Collision2D.Epsilon (Collision2D.cs:44), which is not yet
        // ported (scheduled for Phase 2). Point this at Collision2D::Epsilon once that lands.
        constexpr float kCollision2DEpsilonPending = 1e-6f;
    }

    BoundingCircle2D::BoundingCircle2D(const Vector2& center, const float radius) : Center(center), Radius(radius)
    {
    }

    float BoundingCircle2D::getAreaProperty() const
    {
        return std::numbers::pi_v<float> * Radius * Radius;
    }

    BoundingCircle2D BoundingCircle2D::CreateFromPoints(const std::vector<Vector2>& points)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 4.3.2 "Computing a Bounding Sphere"
        // Implements Ritter's algorithm for approximate bounding sphere computation

        if (points.empty())
        {
            throw std::invalid_argument("Cannot create bounding circle from empty point array");
        }

        if (points.size() == 1)
        {
            return BoundingCircle2D(points[0], 0.0f);
        }

        // Find the most separate point pair along the principle axis
        std::size_t minX = 0, maxX = 0, minY = 0, maxY = 0;

        for (std::size_t i = 1; i < points.size(); i++)
        {
            if (points[i].X < points[minX].X) minX = i;
            if (points[i].X > points[maxX].X) maxX = i;
            if (points[i].Y < points[minY].Y) minY = i;
            if (points[i].Y > points[maxY].Y) maxY = i;
        }

        // Compute squared distances for the two pairs of points
        const Vector2 dx = points[maxX] - points[minX];
        const Vector2 dy = points[maxY] - points[minY];
        const float distSqX = dx.X * dx.X + dx.Y * dx.Y;
        const float distSqY = dy.X * dy.X + dy.Y * dy.Y;

        // Pick the pair of points most distant
        std::size_t min = minX;
        std::size_t max = maxX;
        if (distSqY > distSqX)
        {
            max = maxY;
            min = minY;
        }

        Vector2 center = (points[min] + points[max]) * 0.5f;
        const Vector2 diff = points[max] - center;
        float radius = std::sqrt(diff.X * diff.X + diff.Y * diff.Y);

        // Grow sphere to include all points
        for (const Vector2& point : points)
        {
            const Vector2 d = point - center;
            const float distSq = d.X * d.X + d.Y * d.Y;

            // Only update sphere if point is outside it
            if (distSq > radius * radius)
            {
                const float dist = std::sqrt(distSq);
                const float newRadius = (radius + dist) * 0.5f;
                const float k = (newRadius - radius) / dist;

                radius = newRadius;
                center = center + d * k;
            }
        }

        return BoundingCircle2D(center, radius);
    }

    BoundingCircle2D BoundingCircle2D::CreateFromBoundingBox2D(const BoundingBox2D& box)
    {
        const Vector2 center = box.getCenterProperty();
        const Vector2 halfExtents = box.getHalfExtentsProperty();
        const float radius = halfExtents.Length();

        return BoundingCircle2D(center, radius);
    }

    BoundingCircle2D BoundingCircle2D::CreateFromBoundingCapsule2D(const BoundingCapsule2D& capsule)
    {
        // The bounding circle center is at the capsule's midpoint
        const Vector2 center = capsule.getCenterProperty();

        // The radius needs to reach from the center to the farthest point on either cap:
        // half the capsule length plus the cap radius
        const float radius = (capsule.getLengthProperty() * 0.5f) + capsule.Radius;

        return BoundingCircle2D(center, radius);
    }

    BoundingCircle2D BoundingCircle2D::CreateMerged(const BoundingCircle2D& original, const BoundingCircle2D& additional)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 6.5.2 "Merging Two Spheres" (2D circle adaptation)

        // Calculate distance between centers
        const Vector2 centerDiff = additional.Center - original.Center;
        const float distSq = centerDiff.X * centerDiff.X + centerDiff.Y * centerDiff.Y;
        const float dist = std::sqrt(distSq);

        // Calculate radius difference
        const float radiusDiff = std::abs(additional.Radius - original.Radius);

        // Check if one circle contains the other
        if (radiusDiff >= dist)
        {
            // One circle fully enclosed by the other: return the larger circle
            return additional.Radius >= original.Radius ? additional : original;
        }

        // Circles are partially overlapping or disjoint.
        // Calculate the radius of the new circle as half the maximum distance between surfaces
        const float radius = (dist + original.Radius + additional.Radius) * 0.5f;

        // Calculate the center by adjusting original center toward additional center
        Vector2 center = original.Center;
        if (dist > kCollision2DEpsilonPending)
        {
            center = center + ((radius - original.Radius) / dist) * centerDiff;
        }

        return BoundingCircle2D(center, radius);
    }

    ContainmentType BoundingCircle2D::Contains(const Vector2& point) const
    {
        return Collision2D::ContainsCirclePoint(point, Center, Radius);
    }

    ContainmentType BoundingCircle2D::Contains(const BoundingBox2D& box) const
    {
        return Collision2D::ContainsCircleAabb(Center, Radius, box.Min, box.Max);
    }

    ContainmentType BoundingCircle2D::Contains(const BoundingCircle2D& other) const
    {
        return Collision2D::ContainsCircleCircle(Center, Radius, other.Center, other.Radius);
    }

    ContainmentType BoundingCircle2D::Contains(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::ContainsCircleObb(Center, Radius, obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents);
    }

    ContainmentType BoundingCircle2D::Contains(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::ContainsCircleCapsule(Center, Radius, capsule.PointA, capsule.PointB, capsule.Radius);
    }

    ContainmentType BoundingCircle2D::Contains(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::ContainsCircleConvexPolygon(Center, Radius, polygon.Vertices, polygon.Normals);
    }

    bool BoundingCircle2D::Intersects(const BoundingCircle2D& other) const
    {
        return Collision2D::IntersectsCircleCircle(Center, Radius, other.Center, other.Radius);
    }

    bool BoundingCircle2D::Intersects(const BoundingBox2D& box) const
    {
        return Collision2D::IntersectsCircleAabb(Center, Radius, box.Min, box.Max);
    }

    bool BoundingCircle2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::IntersectsCircleCapsule(Center, Radius, capsule.PointA, capsule.PointB, capsule.Radius);
    }

    bool BoundingCircle2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::IntersectsCircleObb(Center, Radius, obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents);
    }

    bool BoundingCircle2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::IntersectsCircleConvexPolygon(Center, Radius, polygon.Vertices, polygon.Normals);
    }

    bool BoundingCircle2D::TryGetCollision(const BoundingCircle2D& other, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionCircleCircle(Center, Radius, other.Center, other.Radius, result);
    }

    bool BoundingCircle2D::TryGetCollision(const BoundingBox2D& box, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionCircleAabb(Center, Radius, box.Min, box.Max, result);
    }

    bool BoundingCircle2D::TryGetCollision(const BoundingCapsule2D& capsule, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionCircleCapsule(Center, Radius, capsule.PointA, capsule.PointB, capsule.Radius, result);
    }

    bool BoundingCircle2D::TryGetCollision(const OrientedBoundingBox2D& obb, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionCircleObb(Center, Radius, obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents, result);
    }

    BoundingCircle2D BoundingCircle2D::Transform(const Matrix& matrix) const
    {
        BoundingCircle2D circle;
        circle.Center = Vector2::Transform(Center, matrix);

        // Scale radius by maximum scale component
        const float scaleX = std::sqrt(matrix.M11 * matrix.M11 + matrix.M12 * matrix.M12);
        const float scaleY = std::sqrt(matrix.M21 * matrix.M21 + matrix.M22 * matrix.M22);
        circle.Radius = Radius * std::max(scaleX, scaleY);

        return circle;
    }

    BoundingCircle2D BoundingCircle2D::Translate(const Vector2& translation) const
    {
        return BoundingCircle2D(Center + translation, Radius);
    }

    void BoundingCircle2D::Deconstruct(Vector2& center, float& radius) const
    {
        center = Center;
        radius = Radius;
    }

    bool BoundingCircle2D::Equals(const BoundingCircle2D& other) const
    {
        return Center == other.Center && Radius == other.Radius;
    }

    int BoundingCircle2D::GetHashCode() const
    {
        return Center.GetHashCode() ^ static_cast<int>(std::hash<float>{}(Radius));
    }

    std::string BoundingCircle2D::ToString() const
    {
        return "{Center:" + Center.ToString() + " Radius:" + std::to_string(Radius) + "}";
    }
}
