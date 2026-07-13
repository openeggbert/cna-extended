// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Ray2D.cs. Fields, the constructor, CreateFromPoints, GetPoint,
// ClosestPoint (a real algorithm, but pure Vector2 arithmetic -- not Collision2D),
// DistanceSquaredToPoint/DistanceToPoint (both self-contained via ClosestPoint, unlike
// LineSegment2D's equivalents which needed Collision2D), Normalize (3 overloads), Deconstruct,
// and Equals/GetHashCode/ToString/operators were ported before Collision2D existed. Now that
// Collision2D is fully ported, every Intersects(...) overload (against Line2D, Ray2D,
// LineSegment2D, and all 5 bounding-volume types) is ported too: each one delegates to a real
// Collision2D algorithm (SolveParametricIntersection2D, ClipLineToAabb,
// RayCircleIntersectionInterval, RayCapsuleIntersectionInterval, ClipLineToConvexPolygon,
// ClipInterval) or to a sibling method on Line2D. Ray2D.cs is now 100% ported.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class Line2D;
    class LineSegment2D;
    class BoundingBox2D;
    class BoundingCircle2D;
    class BoundingCapsule2D;
    class OrientedBoundingBox2D;
    class BoundingPolygon2D;

    /** @brief Represents a semi-infinite ray in 2D space starting at an origin point and extending in a direction. */
    struct Ray2D
    {
        /**
         * @brief The direction vector defining which way this ray extends from its origin.
         * Should be normalized for accurate distance calculations.
         */
        Vector2 Direction;

        /** @brief The starting point of this ray in 2D space. */
        Vector2 Origin;

        Ray2D() = default;

        /**
         * @brief Creates a new Ray2D with the specified origin and direction.
         * @param origin The starting point of the ray in 2D space.
         * @param direction The direction vector defining which way the ray extends. Should be
         * normalized for accurate distance calculations.
         */
        Ray2D(const Vector2& origin, const Vector2& direction);

        /**
         * @brief Creates a Ray2D from a starting point toward a target point.
         * @param start The starting point of the ray in 2D space.
         * @param through A target point the ray should pass through.
         * @throws std::invalid_argument if the points are too close to define a valid direction.
         */
        [[nodiscard]] static Ray2D CreateFromPoints(const Vector2& start, const Vector2& through);

        /**
         * @brief Computes a point along this ray at the specified parametric distance.
         * @param distanceAlongRay The parametric distance along the ray from the origin.
         */
        [[nodiscard]] Vector2 GetPoint(float distanceAlongRay) const;

        /**
         * @brief Computes the closest point on this ray to a specified point.
         * @param point The point in 2D space to find the closest point to.
         * @param distanceAlongRay Receives the parametric distance along the ray to the closest
         * point, clamped to zero or greater.
         */
        [[nodiscard]] Vector2 ClosestPoint(const Vector2& point, float& distanceAlongRay) const;

        /**
         * @brief Computes the squared distance from a point to the closest point on this ray.
         * @param point The point in 2D space to measure from.
         */
        [[nodiscard]] float DistanceSquaredToPoint(const Vector2& point) const;

        /**
         * @brief Computes the distance from a point to the closest point on this ray.
         * @param point The point in 2D space to measure from.
         */
        [[nodiscard]] float DistanceToPoint(const Vector2& point) const;

        /**
         * @brief Returns a normalized representation of the specified ray with a unit direction
         * vector.
         * @param value The ray to normalize.
         * @param result Receives a Ray2D with the same origin but a unit direction vector.
         */
        static void Normalize(const Ray2D& value, Ray2D& result);

        /**
         * @brief Returns a normalized representation of the specified ray with a unit direction
         * vector.
         * @param value The ray to normalize.
         */
        [[nodiscard]] static Ray2D Normalize(const Ray2D& value);

        /** @brief Normalizes this ray's direction vector to unit length. */
        void Normalize();

        /**
         * @brief Tests if this ray intersects with a line.
         * @param line The line to test against.
         * @param distanceAlongRay Receives the parametric distance along this ray to the
         * intersection point, or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const Line2D& line, std::optional<float>& distanceAlongRay, std::optional<Vector2>& point) const;

        /** @brief Tests if this ray intersects with a line. */
        [[nodiscard]] bool Intersects(const Line2D& line) const;

        /**
         * @brief Tests if this ray intersects with another ray.
         * @param other The other ray to test against.
         * @param distanceAlongRay1 Receives the parametric distance along this ray to the
         * intersection point, or std::nullopt if there is none.
         * @param distanceAlongRay2 Receives the parametric distance along the other ray to the
         * intersection point, or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const Ray2D& other, std::optional<float>& distanceAlongRay1, std::optional<float>& distanceAlongRay2,
            std::optional<Vector2>& point) const;

        /** @brief Tests if this ray intersects with another ray. */
        [[nodiscard]] bool Intersects(const Ray2D& other) const;

        /**
         * @brief Tests if this ray intersects with a line segment.
         * @param segment The line segment to test against.
         * @param distanceAlongRay Receives the parametric distance along this ray to the
         * intersection point, or std::nullopt if there is none.
         * @param distanceAlongSegment Receives the parametric distance along the segment to the
         * intersection point in [0, 1], or std::nullopt if there is none.
         * @param point Receives the intersection point, or std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const LineSegment2D& segment, std::optional<float>& distanceAlongRay,
            std::optional<float>& distanceAlongSegment, std::optional<Vector2>& point) const;

        /** @brief Tests if this ray intersects with a line segment. */
        [[nodiscard]] bool Intersects(const LineSegment2D& segment) const;

        /**
         * @brief Tests if this ray intersects with an axis-aligned bounding box and computes the
         * parametric distances to the intersection points.
         * @param box The bounding box to test against.
         * @param tRayMin Receives the parametric distance to the entry point (0 if the ray origin
         * is inside the box), or std::nullopt if there is none.
         * @param tRayMax Receives the parametric distance to the exit point, or std::nullopt if
         * there is none.
         */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const;

        /** @brief Tests if this ray intersects with an axis-aligned bounding box. */
        [[nodiscard]] bool Intersects(const BoundingBox2D& box) const;

        /**
         * @brief Tests if this ray intersects with a circle and computes the parametric distances
         * to the intersection points.
         * @param circle The circle to test against.
         * @param tRayMin Receives the parametric distance to the first intersection point (0 if
         * the ray origin is inside the circle), or std::nullopt if there is none.
         * @param tRayMax Receives the parametric distance to the second intersection point, or
         * std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const;

        /** @brief Tests if this ray intersects with a circle. */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;

        /**
         * @brief Tests if this ray intersects with a capsule and computes the parametric distances
         * to the intersection points.
         * @param capsule The capsule to test against.
         * @param tRayMin Receives the parametric distance to the entry point (0 if the ray origin
         * is inside the capsule), or std::nullopt if there is none.
         * @param tRayMax Receives the parametric distance to the exit point, or std::nullopt if
         * there is none.
         */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const;

        /** @brief Tests if this ray intersects with a capsule. */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;

        /**
         * @brief Tests if this ray intersects with an oriented bounding box and computes the
         * parametric distances to the intersection points.
         * @param obb The oriented bounding box to test against.
         * @param tRayMin Receives the parametric distance to the entry point (0 if the ray origin
         * is inside the box), or std::nullopt if there is none.
         * @param tRayMax Receives the parametric distance to the exit point, or std::nullopt if
         * there is none.
         */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb, std::optional<float>& tRayMin, std::optional<float>& tRayMax) const;

        /** @brief Tests if this ray intersects with an oriented bounding box. */
        [[nodiscard]] bool Intersects(const OrientedBoundingBox2D& obb) const;

        /**
         * @brief Tests if this ray intersects with a polygon and computes the parametric distances
         * to the intersection points.
         * @param polygon The polygon to test against.
         * @param tRayMin Receives the parametric distance to the entry point (0 if the ray origin
         * is inside the polygon), or std::nullopt if there is none.
         * @param tRayMax Receives the parametric distance to the exit point, or std::nullopt if
         * there is none.
         * @param point Receives the entry intersection point corresponding to tRayMin, or
         * std::nullopt if there is none.
         */
        [[nodiscard]] bool Intersects(
            const BoundingPolygon2D& polygon, std::optional<float>& tRayMin, std::optional<float>& tRayMax, std::optional<Vector2>& point) const;

        /** @brief Tests if this ray intersects with a polygon. */
        [[nodiscard]] bool Intersects(const BoundingPolygon2D& polygon) const;

        /**
         * @brief Deconstructs this ray into its component values.
         * @param origin Receives the starting point.
         * @param direction Receives the direction vector.
         */
        void Deconstruct(Vector2& origin, Vector2& direction) const;

        [[nodiscard]] bool Equals(const Ray2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Ray2D& left, const Ray2D& right) { return left.Equals(right); }
        friend bool operator!=(const Ray2D& left, const Ray2D& right) { return !left.Equals(right); }
    };
}
