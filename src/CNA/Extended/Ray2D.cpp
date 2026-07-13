// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
#include "CNA/Extended/Ray2D.hpp"

#include "CNA/Extended/BoundingBox2D.hpp"
#include "CNA/Extended/BoundingCapsule2D.hpp"
#include "CNA/Extended/BoundingCircle2D.hpp"
#include "CNA/Extended/BoundingPolygon2D.hpp"
#include "CNA/Extended/Collision2D.hpp"
#include "CNA/Extended/Line2D.hpp"
#include "CNA/Extended/LineSegment2D.hpp"
#include "CNA/Extended/OrientedBoundingBox2D.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace CNA::Extended
{
    Ray2D::Ray2D(const Vector2& origin, const Vector2& direction) : Direction(direction), Origin(origin)
    {
    }

    Ray2D Ray2D::CreateFromPoints(const Vector2& start, const Vector2& through)
    {
        const Vector2 direction = through - start;
        const float lengthSq = direction.LengthSquared();

        if (lengthSq < Collision2D::Epsilon * Collision2D::Epsilon)
        {
            throw std::invalid_argument("Points must be distinct to define a ray direction");
        }

        return Ray2D(start, Vector2::Normalize(direction));
    }

    Vector2 Ray2D::GetPoint(const float distanceAlongRay) const
    {
        return Origin + distanceAlongRay * Direction;
    }

    Vector2 Ray2D::ClosestPoint(const Vector2& point, float& distanceAlongRay) const
    {
        const Vector2 ab = Direction;

        const float denom = Vector2::Dot(ab, ab);
        if (denom <= Collision2D::Epsilon)
        {
            // Direction of ray is effectively zero, so it's just a point.
            distanceAlongRay = 0.0f;
            return Origin;
        }

        // Project point ab, but deferring divide by Dot(ab, ab)
        distanceAlongRay = Vector2::Dot(point - Origin, ab);
        if (distanceAlongRay <= 0.0f)
        {
            // point projects before the ray origin, clamp to origin
            distanceAlongRay = 0.0f;
            return Origin;
        }

        // Point projects after the ray origin, must do deferred divide
        distanceAlongRay /= denom;
        return Origin + distanceAlongRay * ab;
    }

    float Ray2D::DistanceSquaredToPoint(const Vector2& point) const
    {
        float distanceAlongRay = 0.0f;
        const Vector2 closestPoint = ClosestPoint(point, distanceAlongRay);
        return Vector2::DistanceSquared(point, closestPoint);
    }

    float Ray2D::DistanceToPoint(const Vector2& point) const
    {
        const float distSq = DistanceSquaredToPoint(point);
        return std::sqrt(distSq);
    }

    void Ray2D::Normalize(const Ray2D& value, Ray2D& result)
    {
        result = Ray2D(value.Origin, Vector2::Normalize(value.Direction));
    }

    Ray2D Ray2D::Normalize(const Ray2D& value)
    {
        Ray2D result;
        Normalize(value, result);
        return result;
    }

    void Ray2D::Normalize()
    {
        Direction = Vector2::Normalize(Direction);
    }

    bool Ray2D::Intersects(const Line2D& line, std::optional<float>& distanceAlongRay, std::optional<Vector2>& point) const
    {
        return line.Intersects(*this, distanceAlongRay, point);
    }

    bool Ray2D::Intersects(const Line2D& line) const
    {
        return line.Intersects(*this);
    }

    bool Ray2D::Intersects(
        const Ray2D& other, std::optional<float>& distanceAlongRay1, std::optional<float>& distanceAlongRay2, std::optional<Vector2>& point) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of two rays using 2D cross products
        // Derived from Section 5.1.9.1 "2D Segment Intersection" (line-line formulation)
        float t1 = 0.0f;
        float t2 = 0.0f;
        if (!Collision2D::SolveParametricIntersection2D(Origin, Direction, other.Origin, other.Direction, t1, t2))
        {
            distanceAlongRay1 = std::nullopt;
            distanceAlongRay2 = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Only intersections in forward direction. Negative direction indicates
        // intersection would have happened behind origin.
        if (t1 < 0.0f || t2 < 0.0f)
        {
            distanceAlongRay1 = std::nullopt;
            distanceAlongRay2 = std::nullopt;
            point = std::nullopt;
            return false;
        }

        distanceAlongRay1 = t1;
        distanceAlongRay2 = t2;
        point = Origin + t1 * Direction;
        return true;
    }

    bool Ray2D::Intersects(const Ray2D& other) const
    {
        std::optional<float> distanceAlongRay1;
        std::optional<float> distanceAlongRay2;
        std::optional<Vector2> point;
        return Intersects(other, distanceAlongRay1, distanceAlongRay2, point);
    }

    bool Ray2D::Intersects(const LineSegment2D& segment, std::optional<float>& distanceAlongRay, std::optional<float>& distanceAlongSegment,
        std::optional<Vector2>& point) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of a ray with a line segment using 2D cross products
        // Derived from Section 5.1.9.1 "2D Segment Intersection" (line-line formulation)
        const Vector2 segmentDir = segment.End - segment.Start;

        float tRay = 0.0f;
        float tSeg = 0.0f;
        if (!Collision2D::SolveParametricIntersection2D(Origin, Direction, segment.Start, segmentDir, tRay, tSeg))
        {
            distanceAlongRay = std::nullopt;
            distanceAlongSegment = std::nullopt;
            point = std::nullopt;
            return false;
        }

        // Intersection only if within segments bounds [0,1] and only in forward direction of ray.
        // Negative direction of ray indicates intersection would have happened behind ray origin
        if (tSeg < 0.0f || tSeg > 1.0f || tRay < 0.0f)
        {
            distanceAlongRay = std::nullopt;
            distanceAlongSegment = std::nullopt;
            point = std::nullopt;
            return false;
        }

        distanceAlongRay = tRay;
        distanceAlongSegment = tSeg;
        point = Origin + tRay * Direction;
        return true;
    }

    bool Ray2D::Intersects(const LineSegment2D& segment) const
    {
        std::optional<float> distanceAlongRay;
        std::optional<float> distanceAlongSegment;
        std::optional<Vector2> point;
        return Intersects(segment, distanceAlongRay, distanceAlongSegment, point);
    }

    bool Ray2D::Intersects(const BoundingBox2D& box, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const
    {
        float tEnter = 0.0f;
        float tExit = 0.0f;
        if (!Collision2D::ClipLineToAabb(Origin, Direction, box.Min, box.Max, 0.0f, std::numeric_limits<float>::max(), tEnter, tExit))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        tRayMin = tEnter;
        tRayMax = tExit;
        return true;
    }

    bool Ray2D::Intersects(const BoundingBox2D& box) const
    {
        std::optional<float> tRayMin;
        std::optional<float> tRayMax;
        return Intersects(box, tRayMin, tRayMax);
    }

    bool Ray2D::Intersects(const BoundingCircle2D& circle, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const
    {
        float tMin = 0.0f;
        float tMax = 0.0f;
        if (!Collision2D::RayCircleIntersectionInterval(Origin, Direction, circle.Center, circle.Radius, tMin, tMax))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        float entry = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipInterval(tMin, tMax, 0.0f, std::numeric_limits<float>::max(), entry, exit))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        tRayMin = entry;
        tRayMax = exit;
        return true;
    }

    bool Ray2D::Intersects(const BoundingCircle2D& circle) const
    {
        std::optional<float> tRayMin;
        std::optional<float> tRayMax;
        return Intersects(circle, tRayMin, tRayMax);
    }

    bool Ray2D::Intersects(const BoundingCapsule2D& capsule, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const
    {
        float tMin = 0.0f;
        float tMax = 0.0f;
        if (!Collision2D::RayCapsuleIntersectionInterval(Origin, Direction, capsule.PointA, capsule.PointB, capsule.Radius, tMin, tMax))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        float entry = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipInterval(tMin, tMax, 0.0f, std::numeric_limits<float>::max(), entry, exit))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        tRayMin = entry;
        tRayMax = exit;
        return true;
    }

    bool Ray2D::Intersects(const BoundingCapsule2D& capsule) const
    {
        std::optional<float> tRayMin;
        std::optional<float> tRayMax;
        return Intersects(capsule, tRayMin, tRayMax);
    }

    bool Ray2D::Intersects(const OrientedBoundingBox2D& obb, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const
    {
        // C. Ericson, Real-Time Collision Detection, Morgan Kaufmann, 2005
        // Parametric intersection of a ray with an oriented bounding rectangle (2D reduction)
        // Derived from Section 5.3.3 "Intersecting Ray or Segment Against Box"

        // Project the ray origin and direction onto the OBB's local axes to transform the ray
        // from world space to OBB local space.
        const Vector2 diff = Origin - obb.Center;

        const Vector2 localOrigin(Vector2::Dot(diff, obb.AxisX), Vector2::Dot(diff, obb.AxisY));
        const Vector2 localDirection(Vector2::Dot(Direction, obb.AxisX), Vector2::Dot(Direction, obb.AxisY));

        float tEnter = 0.0f;
        float tExit = 0.0f;
        if (!Collision2D::ClipLineToAabb(
                localOrigin, localDirection, -obb.HalfExtents, obb.HalfExtents, 0.0f, std::numeric_limits<float>::max(), tEnter, tExit))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            return false;
        }

        tRayMin = tEnter;
        tRayMax = tExit;
        return true;
    }

    bool Ray2D::Intersects(const OrientedBoundingBox2D& obb) const
    {
        std::optional<float> tRayMin;
        std::optional<float> tRayMax;
        return Intersects(obb, tRayMin, tRayMax);
    }

    bool Ray2D::Intersects(
        const BoundingPolygon2D& polygon, std::optional<float>& tRayMin, std::optional<float>& tRayMax, std::optional<Vector2>& point) const
    {
        float t0 = 0.0f;
        float t1 = 0.0f;
        if (!Collision2D::ClipLineToConvexPolygon(
                Origin, Direction, polygon.Vertices, polygon.Normals, 0.0f, std::numeric_limits<float>::max(), t0, t1))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            point = std::nullopt;
            return false;
        }

        float entry = 0.0f;
        float exit = 0.0f;
        if (!Collision2D::ClipInterval(t0, t1, 0.0f, std::numeric_limits<float>::max(), entry, exit))
        {
            tRayMin = std::nullopt;
            tRayMax = std::nullopt;
            point = std::nullopt;
            return false;
        }

        tRayMin = entry;
        tRayMax = exit;
        point = Origin + Direction * entry;
        return true;
    }

    bool Ray2D::Intersects(const BoundingPolygon2D& polygon) const
    {
        std::optional<float> tRayMin;
        std::optional<float> tRayMax;
        std::optional<Vector2> point;
        return Intersects(polygon, tRayMin, tRayMax, point);
    }

    void Ray2D::Deconstruct(Vector2& origin, Vector2& direction) const
    {
        origin = Origin;
        direction = Direction;
    }

    bool Ray2D::Equals(const Ray2D& other) const
    {
        return Origin == other.Origin && Direction == other.Direction;
    }

    int Ray2D::GetHashCode() const
    {
        return Origin.GetHashCode() ^ Direction.GetHashCode();
    }

    std::string Ray2D::ToString() const
    {
        return "{Origin:" + Origin.ToString() + " Direction:" + Direction.ToString() + "}";
    }
}
