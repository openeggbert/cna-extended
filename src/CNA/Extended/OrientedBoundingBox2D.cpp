// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"

#include <cmath>
#include <limits>

namespace CNA::Extended
{
    OrientedBoundingBox2D::OrientedBoundingBox2D(
        const Vector2& center, const Vector2& axisX, const Vector2& axisY, const Vector2& halfExtents)
        : Center(center), AxisX(axisX), AxisY(axisY), HalfExtents(halfExtents)
    {
    }

    float OrientedBoundingBox2D::getRotationProperty() const
    {
        return std::atan2(AxisX.Y, AxisX.X);
    }

    OrientedBoundingBox2D OrientedBoundingBox2D::CreateFromRotation(
        const Vector2& center, const float rotation, const Vector2& halfExtents)
    {
        const float cos = std::cos(rotation);
        const float sin = std::sin(rotation);

        return OrientedBoundingBox2D(center, Vector2(cos, sin), Vector2(-sin, cos), halfExtents);
    }

    OrientedBoundingBox2D OrientedBoundingBox2D::CreateFromBoundingBox2D(const BoundingBox2D& box)
    {
        return OrientedBoundingBox2D(box.getCenterProperty(), Vector2(1, 0), Vector2(0, 1), box.getHalfExtentsProperty());
    }

    OrientedBoundingBox2D OrientedBoundingBox2D::CreateMerged(const OrientedBoundingBox2D& original, const OrientedBoundingBox2D& additional)
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Section 6.5.3 "Merging Two OBBs", Section 4.4.2 "PCA-based OBBs"
        // Derived PCA-based OBB merge using power iteration in place of the
        // Jacobi eigenvalue method described in the book.

        constexpr int Iterations = 4;

        // Get all corners from both rectangles.
        // Computing inline instead of calling GetCorners to avoid multiple array allocations and array copies
        const Vector2 extX1 = original.AxisX * original.HalfExtents.X;
        const Vector2 extY1 = original.AxisY * original.HalfExtents.Y;
        const Vector2 extX2 = additional.AxisX * additional.HalfExtents.X;
        const Vector2 extY2 = additional.AxisY * additional.HalfExtents.Y;
        const Vector2 allCorners[8] = {
            original.Center - extX1 - extY1,
            original.Center + extX1 - extY1,
            original.Center + extX1 + extY1,
            original.Center - extX1 + extY1,

            additional.Center - extX2 - extY2,
            additional.Center + extX2 - extY2,
            additional.Center + extX2 + extY2,
            additional.Center - extX2 + extY2,
        };

        // Compute the centroid
        Vector2 centroid = Vector2::Zero;
        for (const Vector2& corner : allCorners)
        {
            centroid = centroid + corner;
        }
        centroid = centroid / static_cast<float>(std::size(allCorners));

        // Compute covariance matrix
        float cxx = 0, cxy = 0, cyy = 0;
        for (const Vector2& corner : allCorners)
        {
            const Vector2 p = corner - centroid;
            cxx += p.X * p.X;
            cxy += p.X * p.Y;
            cyy += p.Y * p.Y;
        }

        // Find eigenvector of largest eigenvalue using power iteration, to get the primary axis of the OBB
        Vector2 axisX = Vector2::UnitX;
        for (int iter = 0; iter < Iterations; iter++)
        {
            const float newX = cxx * axisX.X + cxy * axisX.Y;
            const float newY = cxy * axisX.X + cyy * axisX.Y;
            axisX = Vector2(newX, newY);

            // Only sqrt if necessary
            const float lenSq = axisX.LengthSquared();
            if (lenSq > Collision2D::EpsilonSq)
            {
                axisX = axisX / std::sqrt(lenSq);
            }
        }

        // Compute perpendicular axis
        const Vector2 axisY(-axisX.Y, axisX.X);

        // Project all points onto both axes to find extents
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();

        for (const Vector2& corner : allCorners)
        {
            const Vector2 p = corner - centroid;
            const float projX = Vector2::Dot(p, axisX);
            const float projY = Vector2::Dot(p, axisY);

            minX = std::min(minX, projX);
            maxX = std::max(maxX, projX);
            minY = std::min(minY, projY);
            maxY = std::max(maxY, projY);
        }

        // Compute center and extents in the new OBB's local space
        const float centerOffsetX = (minX + maxX) * 0.5f;
        const float centerOffsetY = (minY + maxY) * 0.5f;
        const Vector2 center = centroid + axisX * centerOffsetX + axisY * centerOffsetY;

        const Vector2 halfExtents((maxX - minX) * 0.5f, (maxY - minY) * 0.5f);

        return OrientedBoundingBox2D(center, axisX, axisY, halfExtents);
    }

    std::vector<Vector2> OrientedBoundingBox2D::GetCorners() const
    {
        const Vector2 extX = AxisX * HalfExtents.X;
        const Vector2 extY = AxisY * HalfExtents.Y;

        return {
            Center - extX - extY, // Top-left
            Center + extX - extY, // Top-right
            Center + extX + extY, // Bottom-right
            Center - extX + extY, // Bottom-left
        };
    }

    void OrientedBoundingBox2D::GetCorners(Vector2 corners[4]) const
    {
        const Vector2 extX = AxisX * HalfExtents.X;
        const Vector2 extY = AxisY * HalfExtents.Y;

        corners[0] = Center - extX - extY; // Top-left
        corners[1] = Center + extX - extY; // Top-right
        corners[2] = Center + extX + extY; // Bottom-right
        corners[3] = Center - extX + extY; // Bottom-left
    }

    bool OrientedBoundingBox2D::Equals(const OrientedBoundingBox2D& other) const
    {
        return Center == other.Center && AxisX == other.AxisX && AxisY == other.AxisY && HalfExtents == other.HalfExtents;
    }

    int OrientedBoundingBox2D::GetHashCode() const
    {
        return Center.GetHashCode() ^ AxisX.GetHashCode() ^ AxisY.GetHashCode() ^ HalfExtents.GetHashCode();
    }

    OrientedBoundingBox2D OrientedBoundingBox2D::Transform(const Matrix& matrix) const
    {
        const Vector2 transformedCenter = Vector2::Transform(Center, matrix);

        // Transform the axes (rotation and scale)
        Vector2 transformedAxisX = Vector2::TransformNormal(AxisX, matrix);
        Vector2 transformedAxisY = Vector2::TransformNormal(AxisY, matrix);

        // Extract scale from transformed axes
        const float scaleX = transformedAxisX.Length();
        const float scaleY = transformedAxisY.Length();

        // Normalize axes
        transformedAxisX = scaleX > Collision2D::Epsilon ? transformedAxisX / scaleX : Vector2::UnitX;
        transformedAxisY = scaleY > Collision2D::Epsilon ? transformedAxisY / scaleY : Vector2::UnitY;

        // Scale the extents
        const Vector2 transformedExtents(HalfExtents.X * scaleX, HalfExtents.Y * scaleY);

        return OrientedBoundingBox2D(transformedCenter, transformedAxisX, transformedAxisY, transformedExtents);
    }

    OrientedBoundingBox2D OrientedBoundingBox2D::Translate(const Vector2& translation) const
    {
        return OrientedBoundingBox2D(Center + translation, AxisX, AxisY, HalfExtents);
    }

    void OrientedBoundingBox2D::Deconstruct(Vector2& center, Vector2& axisX, Vector2& axisY, Vector2& halfExtents) const
    {
        center = Center;
        axisX = AxisX;
        axisY = AxisY;
        halfExtents = HalfExtents;
    }

    ContainmentType OrientedBoundingBox2D::Contains(const Vector2& point) const
    {
        return Collision2D::ContainsObbPoint(point, Center, AxisX, AxisY, HalfExtents);
    }

    ContainmentType OrientedBoundingBox2D::Contains(const BoundingBox2D& aabb) const
    {
        return Collision2D::ContainsObbAabb(Center, AxisX, AxisY, HalfExtents, aabb.Min, aabb.Max);
    }

    ContainmentType OrientedBoundingBox2D::Contains(const BoundingCircle2D& circle) const
    {
        return Collision2D::ContainsObbCircle(Center, AxisX, AxisY, HalfExtents, circle.Center, circle.Radius);
    }

    ContainmentType OrientedBoundingBox2D::Contains(const OrientedBoundingBox2D& other) const
    {
        return Collision2D::ContainsObbObb(Center, AxisX, AxisY, HalfExtents, other.Center, other.AxisX, other.AxisY, other.HalfExtents);
    }

    ContainmentType OrientedBoundingBox2D::Contains(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::ContainsObbCapsule(Center, AxisX, AxisY, HalfExtents, capsule.PointA, capsule.PointB, capsule.Radius);
    }

    ContainmentType OrientedBoundingBox2D::Contains(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::ContainsObbConvexPolygon(Center, AxisX, AxisY, HalfExtents, polygon.Vertices, polygon.Normals);
    }

    bool OrientedBoundingBox2D::Intersects(const BoundingCircle2D& circle) const
    {
        return Collision2D::IntersectsCircleObb(circle.Center, circle.Radius, Center, AxisX, AxisY, HalfExtents);
    }

    bool OrientedBoundingBox2D::Intersects(const OrientedBoundingBox2D& other) const
    {
        return Collision2D::IntersectsObbObb(Center, AxisX, AxisY, HalfExtents, other.Center, other.AxisX, other.AxisY, other.HalfExtents);
    }

    bool OrientedBoundingBox2D::Intersects(const BoundingBox2D& box) const
    {
        return Collision2D::IntersectsAabbObb(box.getCenterProperty(), box.getHalfExtentsProperty(), Center, AxisX, AxisY, HalfExtents);
    }

    bool OrientedBoundingBox2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        return Collision2D::IntersectsObbCapsule(Center, AxisX, AxisY, HalfExtents, capsule.PointA, capsule.PointB, capsule.Radius);
    }

    bool OrientedBoundingBox2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        return Collision2D::IntersectsObbConvexPolygon(Center, AxisX, AxisY, HalfExtents, polygon.Vertices, polygon.Normals);
    }

    bool OrientedBoundingBox2D::TryGetCollision(const BoundingCircle2D& circle, CollisionResult2D& result) const
    {
        CollisionResult2D circleResult;
        if (!Collision2D::TryGetCollisionCircleObb(circle.Center, circle.Radius, Center, AxisX, AxisY, HalfExtents, circleResult))
        {
            result = CollisionResult2D::None;
            return false;
        }

        result = circleResult.Invert();
        return true;
    }

    bool OrientedBoundingBox2D::TryGetCollision(const OrientedBoundingBox2D& other, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionObbObb(
            Center, AxisX, AxisY, HalfExtents, other.Center, other.AxisX, other.AxisY, other.HalfExtents, result);
    }

    bool OrientedBoundingBox2D::TryGetCollision(const BoundingBox2D& box, CollisionResult2D& result) const
    {
        CollisionResult2D boxResult;
        if (!Collision2D::TryGetCollisionAabbObb(box.getCenterProperty(), box.getHalfExtentsProperty(), Center, AxisX, AxisY, HalfExtents, boxResult))
        {
            result = CollisionResult2D::None;
            return false;
        }

        result = boxResult.Invert();
        return true;
    }

    bool OrientedBoundingBox2D::TryGetCollision(const BoundingPolygon2D& polygon, CollisionResult2D& result) const
    {
        return Collision2D::TryGetCollisionObbConvexPolygon(Center, AxisX, AxisY, HalfExtents, polygon.Vertices, polygon.Normals, result);
    }

    std::string OrientedBoundingBox2D::ToString() const
    {
        return "{Center:" + Center.ToString() + " Rotation:" + std::to_string(getRotationProperty()) + " Size:" +
               std::to_string(getWidthProperty()) + "x" + std::to_string(getHeightProperty()) + "}";
    }
}
