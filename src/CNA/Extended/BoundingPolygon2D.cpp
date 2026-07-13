// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/BoundingPolygon2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"
#include "System/HashCode.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

namespace CNA::Extended
{
    namespace
    {
        // Duplicates MonoGame.Extended's Collision2D.Epsilon (Collision2D.cs:44), which is not yet
        // ported (scheduled for Phase 2). Point this at Collision2D::Epsilon once that lands.
        constexpr float kCollision2DEpsilonPending = 1e-6f;
    }

    BoundingPolygon2D::BoundingPolygon2D(std::vector<Vector2> vertices) : Vertices(std::move(vertices))
    {
        if (Vertices.size() < 3)
        {
            throw std::invalid_argument("Polygon must have at least 3 vertices");
        }

        assert(ComputeSignedArea(Vertices) > 1e-6f &&
               "Vertices must be in counter-clockwise order. Ensure vertices follow the documented winding order.");

        Normals = ComputeNormals(Vertices);
    }

    BoundingPolygon2D::BoundingPolygon2D(std::vector<Vector2> vertices, std::vector<Vector2> normals)
        : Vertices(std::move(vertices)), Normals(std::move(normals))
    {
        if (Vertices.size() < 3)
        {
            throw std::invalid_argument("Polygon must have at least 3 vertices");
        }

        if (Vertices.size() != Normals.size())
        {
            throw std::invalid_argument("Normals array must have same length as vertices array");
        }
    }

    Vector2 BoundingPolygon2D::getCentroidProperty() const
    {
        if (Vertices.empty())
        {
            return Vector2::Zero;
        }

        Vector2 sum = Vector2::Zero;
        for (const Vector2& vertex : Vertices)
        {
            sum = sum + vertex;
        }

        return sum / static_cast<float>(Vertices.size());
    }

    float BoundingPolygon2D::getAreaProperty() const
    {
        if (Vertices.empty())
        {
            return 0.0f;
        }

        float area = 0.0f;
        const std::size_t n = Vertices.size();

        for (std::size_t i = 0; i < n; i++)
        {
            const std::size_t j = (i + 1) % n;
            area += Vertices[i].X * Vertices[j].Y;
            area -= Vertices[j].X * Vertices[i].Y;
        }

        return std::abs(area) * 0.5f;
    }

    BoundingPolygon2D BoundingPolygon2D::CreateFromVertices(std::vector<Vector2> vertices)
    {
        return BoundingPolygon2D(std::move(vertices));
    }

    BoundingPolygon2D BoundingPolygon2D::CreateRegular(const Vector2& center, const float radius, const int sides, const float rotation)
    {
        if (sides < 3)
        {
            throw std::invalid_argument("Regular polygon must have at least 3 sides");
        }

        std::vector<Vector2> vertices(static_cast<std::size_t>(sides));
        const float angleStep = 6.28318548f / static_cast<float>(sides); // MathHelper::TwoPi

        for (int i = 0; i < sides; i++)
        {
            const float angle = static_cast<float>(i) * angleStep + rotation;
            vertices[static_cast<std::size_t>(i)] = center + Vector2(std::cos(angle) * radius, std::sin(angle) * radius);
        }

        return BoundingPolygon2D(std::move(vertices));
    }

    BoundingPolygon2D BoundingPolygon2D::CreateFromBoundingBox2D(const BoundingBox2D& box)
    {
        std::vector<Vector2> vertices = {
            Vector2(box.Min.X, box.Min.Y), // Top-left
            Vector2(box.Max.X, box.Min.Y), // Top-right
            Vector2(box.Max.X, box.Max.Y), // Bottom-right
            Vector2(box.Min.X, box.Max.Y), // Bottom-left
        };

        return BoundingPolygon2D(std::move(vertices));
    }

    BoundingPolygon2D BoundingPolygon2D::CreateMerged(const BoundingPolygon2D& original, const BoundingPolygon2D& additional)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 4.2.3 "Computing the Convex Hull"
        // Merges polygons by computing the convex hull of the combined vertex set.

        if (original.Vertices.empty())
        {
            return additional;
        }

        if (additional.Vertices.empty())
        {
            return original;
        }

        // Combine all vertices
        std::vector<Vector2> allVertices;
        allVertices.reserve(original.Vertices.size() + additional.Vertices.size());
        allVertices.insert(allVertices.end(), original.Vertices.begin(), original.Vertices.end());
        allVertices.insert(allVertices.end(), additional.Vertices.begin(), additional.Vertices.end());

        // Compute convex hull using Graham scan
        std::vector<Vector2> hullVertices = ComputeConvexHull(std::move(allVertices));

        // Ensure output is ccw order
        EnsureCounterClockwise(hullVertices);

        return BoundingPolygon2D(std::move(hullVertices));
    }

    ContainmentType BoundingPolygon2D::Contains(const Vector2& point) const
    {
        return Collision2D::ContainsConvexPolygonPoint(point, Vertices, Normals);
    }

    ContainmentType BoundingPolygon2D::Contains(const BoundingBox2D& aabb) const
    {
        return Collision2D::ContainsConvexPolygonAabb(Vertices, Normals, aabb.Min, aabb.Max);
    }

    ContainmentType BoundingPolygon2D::Contains(const BoundingCircle2D& circle) const
    {
        return Collision2D::ContainsConvexPolygonCircle(Vertices, Normals, circle.Center, circle.Radius);
    }

    ContainmentType BoundingPolygon2D::Contains(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::ContainsConvexPolygonObb(Vertices, Normals, obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents);
    }

    ContainmentType BoundingPolygon2D::Contains(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::ContainsConvexPolygonCapsule(Vertices, Normals, capsule.PointA, capsule.PointB, capsule.Radius);
    }

    ContainmentType BoundingPolygon2D::Contains(const BoundingPolygon2D& other) const
    {
        return Collision2D::ContainsConvexPolygonConvexPolygon(Vertices, Normals, other.Vertices, other.Normals);
    }

    bool BoundingPolygon2D::Intersects(const BoundingCircle2D& circle) const
    {
        return Collision2D::IntersectsCircleConvexPolygon(circle.Center, circle.Radius, Vertices, Normals);
    }

    bool BoundingPolygon2D::Intersects(const BoundingBox2D& box) const
    {
        return Collision2D::IntersectsAabbConvexPolygon(box.getCenterProperty(), box.getHalfExtentsProperty(), Vertices, Normals);
    }

    bool BoundingPolygon2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        return Collision2D::IntersectsObbConvexPolygon(obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents, Vertices, Normals);
    }

    bool BoundingPolygon2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::IntersectsCapsuleConvexPolygon(capsule.PointA, capsule.PointB, capsule.Radius, Vertices, Normals);
    }

    bool BoundingPolygon2D::Intersects(const BoundingPolygon2D& other) const
    {
        return Collision2D::IntersectsConvexPolygonConvexPolygon(Vertices, Normals, other.Vertices, other.Normals);
    }

    bool BoundingPolygon2D::TryGetCollision(const BoundingBox2D& box, CollisionResult2D& result) const
    {
        CollisionResult2D boxResult;
        if (!Collision2D::TryGetCollisionAabbConvexPolygon(box.getCenterProperty(), box.getHalfExtentsProperty(), Vertices, Normals, boxResult))
        {
            result = CollisionResult2D::None;
            return false;
        }

        result = boxResult.Invert();
        return true;
    }

    bool BoundingPolygon2D::TryGetCollision(const OrientedBoundingBox2D& obb, CollisionResult2D& result) const
    {
        CollisionResult2D obbResult;
        if (!Collision2D::TryGetCollisionObbConvexPolygon(obb.Center, obb.AxisX, obb.AxisY, obb.HalfExtents, Vertices, Normals, obbResult))
        {
            result = CollisionResult2D::None;
            return false;
        }

        result = obbResult.Invert();
        return true;
    }

    bool BoundingPolygon2D::TryGetCollision(const BoundingPolygon2D& other, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionConvexPolygonConvexPolygon(Vertices, Normals, other.Vertices, other.Normals, result);
    }

    BoundingPolygon2D BoundingPolygon2D::Transform(const Matrix& matrix) const
    {
        if (Vertices.empty())
        {
            return *this;
        }

        std::vector<Vector2> transformedVertices(Vertices.size());
        for (std::size_t i = 0; i < Vertices.size(); i++)
        {
            transformedVertices[i] = Vector2::Transform(Vertices[i], matrix);
        }

        return BoundingPolygon2D(std::move(transformedVertices));
    }

    BoundingPolygon2D BoundingPolygon2D::Translate(const Vector2& translation) const
    {
        if (Vertices.empty())
        {
            return *this;
        }

        std::vector<Vector2> translatedVertices(Vertices.size());
        for (std::size_t i = 0; i < Vertices.size(); i++)
        {
            translatedVertices[i] = Vertices[i] + translation;
        }

        // Normals don't change with translation
        return BoundingPolygon2D(std::move(translatedVertices), Normals);
    }

    void BoundingPolygon2D::Deconstruct(std::vector<Vector2>& vertices, std::vector<Vector2>& normals) const
    {
        vertices = Vertices;
        normals = Normals;
    }

    bool BoundingPolygon2D::Equals(const BoundingPolygon2D& other) const
    {
        if (getVertexCountProperty() != other.getVertexCountProperty())
        {
            return false;
        }

        for (std::size_t i = 0; i < Vertices.size(); i++)
        {
            if (!(Vertices[i] == other.Vertices[i]))
            {
                return false;
            }
        }

        return true;
    }

    int BoundingPolygon2D::GetHashCode() const
    {
        if (Vertices.empty())
        {
            return 0;
        }

        // System::HashCode::Add<T> hashes via std::hash<T>, and no std::hash<Vector2> specialization
        // exists (matching how C#'s HashCode.Add<T> itself ultimately calls value.GetHashCode()
        // internally). Add Vector2::GetHashCode()'s result directly instead.
        System::HashCode hash;
        for (const Vector2& vertex : Vertices)
        {
            hash.Add(vertex.GetHashCode());
        }

        return hash.ToHashCode();
    }

    std::string BoundingPolygon2D::ToString() const
    {
        return "{Vertices:" + std::to_string(getVertexCountProperty()) + " Area:" + std::to_string(getAreaProperty()) + "}";
    }

    float BoundingPolygon2D::ComputeSignedArea(const std::vector<Vector2>& vertices)
    {
        if (vertices.size() < 3)
        {
            return 0.0f;
        }

        float area = 0.0f;
        const std::size_t n = vertices.size();

        for (std::size_t i = 0; i < n; i++)
        {
            const std::size_t j = (i + 1) % n;
            // Vector2Extensions.PerpDot(a, b) = a.X*b.Y - a.Y*b.X (Math/Vector2Extensions.cs) --
            // inlined here, see this file's header comment.
            area += (vertices[i].X * vertices[j].Y) - (vertices[i].Y * vertices[j].X);
        }

        return area * 0.5f;
    }

    void BoundingPolygon2D::EnsureCounterClockwise(std::vector<Vector2>& vertices)
    {
        if (vertices.size() < 3)
        {
            return;
        }

        if (ComputeSignedArea(vertices) < 0.0f)
        {
            std::reverse(vertices.begin(), vertices.end());
        }
    }

    std::vector<Vector2> BoundingPolygon2D::ComputeConvexHull(std::vector<Vector2> points)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 4.2.3 "Computing the Convex Hull"
        // Graham scan algorithm adapted for 2D Vector2 input

        if (points.size() < 3)
        {
            return points;
        }

        // Find the point with the lowest Y coordinate, and if tied, the lowest X coordinate as well
        std::size_t minIndex = 0;
        for (std::size_t i = 1; i < points.size(); i++)
        {
            if (points[i].Y < points[minIndex].Y || (points[i].Y == points[minIndex].Y && points[i].X < points[minIndex].X))
            {
                minIndex = i;
            }
        }

        const Vector2 pivot = points[minIndex];

        // Sort points by polar angle with respect to pivot
        std::vector<Vector2> sorted = points;
        std::sort(sorted.begin(), sorted.end(), [pivot](const Vector2& a, const Vector2& b) {
            if (a == pivot) return true;
            if (b == pivot) return false;

            const float angleA = std::atan2(a.Y - pivot.Y, a.X - pivot.X);
            const float angleB = std::atan2(b.Y - pivot.Y, b.X - pivot.X);

            if (std::abs(angleA - angleB) < kCollision2DEpsilonPending)
            {
                // Same angle, closer point comes first
                const float distA = Vector2::DistanceSquared(pivot, a);
                const float distB = Vector2::DistanceSquared(pivot, b);
                return distA > distB;
            }

            return angleA < angleB;
        });

        // Graham scan
        std::vector<Vector2> hull(sorted.size());
        std::size_t hullSize = 0;

        for (const Vector2& point : sorted)
        {
            // Remove points that make a right turn
            while (hullSize >= 2 && Orientation(hull[hullSize - 2], hull[hullSize - 1], point) <= 0)
            {
                hullSize--;
            }

            hull[hullSize++] = point;
        }

        hull.resize(hullSize);
        return hull;
    }

    float BoundingPolygon2D::Orientation(const Vector2& a, const Vector2& b, const Vector2& c)
    {
        return (b.X - a.X) * (c.Y - a.Y) - (b.Y - a.Y) * (c.X - a.X);
    }

    std::vector<Vector2> BoundingPolygon2D::ComputeNormals(const std::vector<Vector2>& vertices)
    {
        const std::size_t n = vertices.size();
        std::vector<Vector2> normals(n);

        for (std::size_t i = 0; i < n; i++)
        {
            const std::size_t j = (i + 1) % n;
            const Vector2 edge = vertices[j] - vertices[i];

            // Perpendicular to edge (rotated 90 degrees clockwise for outward normal)
            normals[i] = Vector2(edge.Y, -edge.X);

            // Normalize
            const float length = normals[i].Length();
            if (length > kCollision2DEpsilonPending)
            {
                normals[i] = normals[i] / length;
            }
        }

        return normals;
    }
}
