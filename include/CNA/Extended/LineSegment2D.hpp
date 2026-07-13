// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's LineSegment2D.cs. Fields, Direction/Midpoint/Length/
// LengthSquared, the constructor, GetBounds(), GetPoint(...), ClosestPoint(...) (a real
// algorithm, but one that only uses Vector2 arithmetic, not Collision2D), Deconstruct,
// Equals/GetHashCode/ToString/operators (Equals here is exact field equality, unlike Line2D's
// epsilon-based Equals -- matches upstream, which does not use Collision2D.Epsilon in this
// type's Equals) were ported before Collision2D existed.
//
// Now that Collision2D is fully ported, every other deferred member except one is ported too:
//   - DistanceSquaredToPoint/DistanceToPoint -> Collision2D.DistanceSquaredPointSegment
//   - DistanceSquaredToSegment/DistanceToSegment -> Collision2D.DistanceSquaredSegmentSegment
//   - Intersects(Line2D, ...) -> Line2D::Intersects(LineSegment2D, ...)
//   - Intersects(Ray2D, ...) -> Ray2D::Intersects(LineSegment2D, ...)
//   - Intersects(LineSegment2D, ...) -> Collision2D.SolveParametricIntersection2D
//   - Intersects(BoundingBox2D/BoundingCircle2D/BoundingCapsule2D/OrientedBoundingBox2D, ...) ->
//     Collision2D.ClipLineToAabb / RayCircleIntersectionInterval / RayCapsuleIntersectionInterval
//
// STILL DEFERRED: Intersects(BoundingPolygon2D, ...) (both the 3-out-param and bool-only
// overloads). Its degenerate (zero-length-segment) branch calls polygon.Contains(Start), and
// BoundingPolygon2D::Contains(...) is itself still deferred pending its own Collision2D-dependent
// follow-up (see BoundingPolygon2D.hpp) -- this is a genuinely new, narrower blocker than
// "Collision2D doesn't exist yet": every other line in this method's body only needs
// already-ported Collision2D methods and BoundingPolygon2D's already-public Vertices/Normals
// fields. Port these two overloads once BoundingPolygon2D::Contains lands.
//
// IMPORTANT correction to BoundingCapsule2D.hpp's deferred-method comment (written when
// BoundingCapsule2D was ported, before this file existed): it assumed CreateFromSegment/
// CreateMerged would become portable once LineSegment2D landed, then a later correction said that
// was wrong because DistanceSquaredToPoint itself needed Collision2D. DistanceSquaredToPoint is
// ported below now, so that second blocker is resolved too -- CreateFromSegment/CreateMerged can
// be ported once BoundingCapsule2D.hpp itself is revisited (out of scope here; BoundingCapsule2D.hpp
// is untouched by this change). See NEXT.md.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class Line2D;
    class Ray2D;
    class BoundingBox2D;
    class BoundingCircle2D;
    class BoundingCapsule2D;
    class OrientedBoundingBox2D;

    /** @brief Represents a finite line segment connecting two endpoints in 2D space. */
    struct LineSegment2D
    {
        /** @brief The starting point of this line segment in 2D space. */
        Vector2 Start;

        /** @brief The ending point of this line segment in 2D space. */
        Vector2 End;

        LineSegment2D() = default;

        /**
         * @brief Creates a new LineSegment2D connecting two specified points.
         * @param start The starting point of the line segment in 2D space.
         * @param end The ending point of the line segment in 2D space.
         */
        LineSegment2D(const Vector2& start, const Vector2& end);

        /**
         * @brief Gets the unnormalized direction vector from start to end. The length of this
         * vector equals the length of the segment.
         */
        [[nodiscard]] Vector2 getDirectionProperty() const { return End - Start; }

        /** @brief Gets the midpoint of this line segment, located halfway between the start and end points. */
        [[nodiscard]] Vector2 getMidpointProperty() const { return (Start + End) * 0.5f; }

        /**
         * @brief Gets the length of this line segment. For length comparisons, use
         * getLengthSquaredProperty() to avoid the square root calculation.
         */
        [[nodiscard]] float getLengthProperty() const;

        /** @brief Gets the squared length of this line segment. */
        [[nodiscard]] float getLengthSquaredProperty() const;

        /**
         * @brief Computes the smallest axis-aligned bounding box that contains this line segment.
         */
        [[nodiscard]] BoundingBox2D GetBounds() const;

        /**
         * @brief Computes a point along this line segment at the specified parametric distance.
         * @param distanceAlongSegment The parametric distance along the segment from the start
         * point, where 0 represents the start, 1 represents the end.
         */
        [[nodiscard]] Vector2 GetPoint(float distanceAlongSegment) const;

        /**
         * @brief Computes the closest point on this line segment to a specified point.
         * @param point The point in 2D space to find the closest point to.
         * @param distanceAlongSegment Receives the parametric distance along the segment to the
         * closest point, clamped to [0, 1].
         */
        [[nodiscard]] Vector2 ClosestPoint(const Vector2& point, float& distanceAlongSegment) const;

        /**
         * @brief Computes the squared distance from a point to the closest point on this line
         * segment.
         * @param point The point in 2D space to measure from.
         */
        [[nodiscard]] float DistanceSquaredToPoint(const Vector2& point) const;

        /**
         * @brief Computes the distance from a point to the closest point on this line segment.
         * @param point The point in 2D space to measure from.
         */
        [[nodiscard]] float DistanceToPoint(const Vector2& point) const;

        /**
         * @brief Computes the squared distance between this line segment and another line
         * segment.
         * @param other The other line segment to measure distance to.
         * @param distanceAlongSegment1 Receives the parametric distance along this segment to the
         * closest point, in [0, 1].
         * @param distanceAlongSegment2 Receives the parametric distance along the other segment
         * to the closest point, in [0, 1].
         * @param closestPoint1 Receives the closest point on this segment to the other segment.
         * @param closestPoint2 Receives the closest point on the other segment to this segment.
         */
        [[nodiscard]] float DistanceSquaredToSegment(const LineSegment2D& other, float& distanceAlongSegment1, float& distanceAlongSegment2,
            Vector2& closestPoint1, Vector2& closestPoint2) const;

        /**
         * @brief Computes the distance between this line segment and another line segment.
         * @param other The other line segment to measure distance to.
         */
        [[nodiscard]] float DistanceToSegment(const LineSegment2D& other) const;

        /**
         * @brief Tests if this line segment intersects with a line.
         * @param line The line to test against.
         * @param distanceAlongSegment Receives the parametric distance along this segment to the
         * intersection point in [0, 1], or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const Line2D& line, std::optional<float>& distanceAlongSegment, std::optional<Vector2>& point) const;

        /** @brief Tests if this line segment intersects with a line. */
        [[nodiscard]] bool Intersects(const Line2D& line) const;

        /**
         * @brief Tests if this line segment intersects with a ray.
         * @param ray The ray to test against.
         * @param distanceAlongSegment Receives the parametric distance along this segment to the
         * intersection point in [0, 1], or std::nullopt if there is none.
         * @param distanceAlongRay Receives the parametric distance along the ray to the
         * intersection point, or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const Ray2D& ray, std::optional<float>& distanceAlongSegment, std::optional<float>& distanceAlongRay,
            std::optional<Vector2>& point) const;

        /** @brief Tests if this line segment intersects with a ray. */
        [[nodiscard]] bool Intersects(const Ray2D& ray) const;

        /**
         * @brief Tests if this line segment intersects with another line segment.
         * @param other The other line segment to test against.
         * @param distanceAlongSegment1 Receives the parametric distance along this segment to the
         * intersection point in [0, 1], or std::nullopt if there is none.
         * @param distanceAlongSegment2 Receives the parametric distance along the other segment
         * to the intersection point in [0, 1], or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const LineSegment2D& other, std::optional<float>& distanceAlongSegment1,
            std::optional<float>& distanceAlongSegment2, std::optional<Vector2>& point) const;

        /** @brief Tests if this line segment intersects with another line segment. */
        [[nodiscard]] bool Intersects(const LineSegment2D& other) const;

        /**
         * @brief Tests if this line segment intersects with an axis-aligned bounding box and
         * computes the parametric distances to the intersection points.
         * @param box The bounding box to test against.
         * @param tMin Receives the parametric distance to the entry point in [0, 1], or
         * std::nullopt if there is none.
         * @param tMax Receives the parametric distance to the exit point in [0, 1], or
         * std::nullopt if there is none.
         * @remark For degenerate (zero-length) segments, returns true with tMin = tMax = 0 if the
         * start point is inside the bounding box.
         */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box, std::optional<float>& tMin, std::optional<float>& tMax) const;

        /** @brief Tests if this line segment intersects with an axis-aligned bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box) const;

        /**
         * @brief Tests if this line segment intersects with a circle and computes the parametric
         * distances to the intersection points.
         * @param circle The circle to test against.
         * @param tSegmentMin Receives the parametric distance to the entry point in [0, 1], or
         * std::nullopt if there is none.
         * @param tSegmentMax Receives the parametric distance to the exit point in [0, 1], or
         * std::nullopt if there is none.
         * @remark For degenerate (zero-length) segments, returns true with tSegmentMin =
         * tSegmentMax = 0 if the start point is inside the circle.
         */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle, std::optional<float>& tSegmentMin, std::optional<float>& tSegmentMax) const;

        /** @brief Tests if this line segment intersects with a circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;

        /**
         * @brief Tests if this line segment intersects with a capsule and computes the parametric
         * distances to the intersection points.
         * @param capsule The capsule to test against.
         * @param tMin Receives the parametric distance to the entry point in [0, 1], or
         * std::nullopt if there is none.
         * @param tMax Receives the parametric distance to the exit point in [0, 1], or
         * std::nullopt if there is none.
         * @remark For degenerate (zero-length) segments, returns true with tMin = tMax = 0 if the
         * start point is inside the capsule.
         */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule, std::optional<float>& tMin, std::optional<float>& tMax) const;

        /** @brief Tests if this line segment intersects with a capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;

        /**
         * @brief Tests if this line segment intersects with an oriented bounding box and computes
         * the parametric distances to the intersection points.
         * @param obb The oriented bounding box to test against.
         * @param tMin Receives the parametric distance to the entry point in [0, 1], or
         * std::nullopt if there is none.
         * @param tMax Receives the parametric distance to the exit point in [0, 1], or
         * std::nullopt if there is none.
         * @remark For degenerate (zero-length) segments, returns true with tMin = tMax = 0 if the
         * start point is inside the box.
         */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb, std::optional<float>& tMin, std::optional<float>& tMax) const;

        /** @brief Tests if this line segment intersects with an oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb) const;

        /**
         * @brief Deconstructs this line segment into its component values.
         * @param start Receives the starting point.
         * @param end Receives the ending point.
         */
        void Deconstruct(Vector2& start, Vector2& end) const;

        [[nodiscard]] bool Equals(const LineSegment2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const LineSegment2D& left, const LineSegment2D& right) { return left.Equals(right); }
        friend bool operator!=(const LineSegment2D& left, const LineSegment2D& right) { return !left.Equals(right); }
    };
}
