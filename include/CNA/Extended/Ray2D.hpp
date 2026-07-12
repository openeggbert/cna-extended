// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Ray2D.cs. Every Intersects(...) overload (against Line2D,
// Ray2D, LineSegment2D, and all 5 bounding-volume types) is deferred to Phase 2 ("Collisions
// 2D"): each one delegates either to a real Collision2D algorithm (SolveParametricIntersection2D,
// ClipLineToAabb, RayCircleIntersectionInterval, RayCapsuleIntersectionInterval,
// ClipLineToConvexPolygon, ClipInterval) or to a deferred sibling method (Line2D::Intersects(Ray2D),
// itself deferred in Line2D.hpp for the same reason). None of that math lives in Ray2D itself.
//
// What IS fully self-contained and ported here: fields, the constructor, CreateFromPoints,
// GetPoint, ClosestPoint (a real algorithm, but pure Vector2 arithmetic -- not Collision2D),
// DistanceSquaredToPoint/DistanceToPoint (both self-contained via ClosestPoint, unlike
// LineSegment2D's equivalents which needed Collision2D), Normalize (3 overloads), Deconstruct,
// Equals/GetHashCode/ToString/operators.
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

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
