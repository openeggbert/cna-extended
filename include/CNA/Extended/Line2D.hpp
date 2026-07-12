// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's Line2D.cs. Upstream's Intersects(...) overloads are split
// between two groups:
//   - Self-contained (ported here): Intersects(Line2D, out Vector2? point) / Intersects(Line2D),
//     Intersects(BoundingCircle2D), Intersects(BoundingCapsule2D) -- these only use
//     DistanceToPoint (self-contained) and/or the trivial Collision2D.Epsilon constant
//     (duplicated locally, see Line2D.cpp), not any real Collision2D algorithm.
//   - Deferred until Collision2D is ported (Phase 2): Intersects(Ray2D, ...) x2,
//     Intersects(LineSegment2D, ...) x2, Intersects(BoundingBox2D), Intersects(OrientedBoundingBox2D),
//     Intersects(BoundingPolygon2D) -- these delegate to real Collision2D algorithms
//     (SolveParametricIntersectionWithImplicitLine, ClipLineToAabb, ClipLineToConvexPolygon).
// C#'s `object obj` overload of Equals has no C++ equivalent (no universal object base) and is
// not ported, matching the precedent set by the bounding-volume types (which only port the
// strongly-typed Equals(T)).
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <optional>
#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class Ray2D;
    class LineSegment2D;
    class BoundingBox2D;
    class BoundingCircle2D;
    class BoundingCapsule2D;
    class OrientedBoundingBox2D;
    class BoundingPolygon2D;

    /** @brief Represents an infinite line in 2D space that extends in both directions without bounds. */
    struct Line2D
    {
        /**
         * @brief The signed perpendicular distance from the origin to this line along its normal
         * direction. Positive values indicate the origin is on the opposite side of the line from
         * the normal.
         */
        float Distance = 0.0f;

        /**
         * @brief The unit normal vector perpendicular to this line, used with Distance to define
         * the line's position and orientation.
         */
        Vector2 Normal;

        Line2D() = default;

        /**
         * @brief Creates a new Line2D with the specified normal and distance from the origin.
         * @param normal The unit normal vector perpendicular to the line.
         * @param distance The perpendicular distance from the origin to the line along its normal
         * direction.
         */
        Line2D(const Vector2& normal, float distance);

        /**
         * @brief Creates a Line2D that passes through a specified point with a given normal
         * direction.
         * @param point A point in 2D space that the line passes through.
         * @param normal The normal vector perpendicular to the line. Normalized automatically.
         */
        [[nodiscard]] static Line2D CreateFromPointAndNormal(const Vector2& point, const Vector2& normal);

        /**
         * @brief Creates a Line2D that passes through two specified points.
         * @param p1 The first point in 2D space.
         * @param p2 The second point in 2D space, must be distinct from the first.
         * @throws std::invalid_argument if the two points are too close to define a unique line.
         */
        [[nodiscard]] static Line2D CreateFromTwoPoints(const Vector2& p1, const Vector2& p2);

        /**
         * @brief Creates a Line2D that passes through a specified point and extends in a given
         * direction.
         * @param point A point in 2D space that the line passes through.
         * @param direction The direction vector the line extends along. Normalized automatically.
         */
        [[nodiscard]] static Line2D CreateFromPointAndDirection(const Vector2& point, const Vector2& direction);

        /**
         * @brief Computes the signed perpendicular distance from a point to this line.
         * @param point The point in 2D space to measure from.
         */
        [[nodiscard]] float DistanceToPoint(const Vector2& point) const;

        /**
         * @brief Computes the closest point on this line to a specified point.
         * @param point The point in 2D space to project onto the line.
         * @param distanceAlongLine Receives the parametric distance along the line's direction
         * vector to the closest point.
         */
        [[nodiscard]] Vector2 ClosestPoint(const Vector2& point, float& distanceAlongLine) const;

        /**
         * @brief Returns a normalized representation of the specified line with a unit normal
         * vector.
         * @param line The line to normalize.
         */
        [[nodiscard]] static Line2D Normalize(const Line2D& line);

        /**
         * @brief Returns a normalized representation of the specified line with a unit normal
         * vector.
         * @param value The line to normalize.
         * @param result Receives a Line2D representing the same geometric line with a unit normal
         * vector and proportionally adjusted distance.
         */
        static void Normalize(const Line2D& value, Line2D& result);

        /** @brief Normalizes this line's representation by ensuring the normal vector has unit length. */
        void Normalize();

        /**
         * @brief Tests if this line intersects with another line.
         * @param other The other line to test against.
         * @param point Receives the intersection point if the lines intersect; std::nullopt if
         * they are parallel or coincident.
         */
        [[nodiscard]] bool Intersects(const Line2D& other, std::optional<Vector2>& point) const;

        /** @brief Tests if this line intersects with another line. */
        [[nodiscard]] bool Intersects(const Line2D& other) const;

        /**
         * @brief Tests if this line intersects with a circle.
         * @param circle The circle to test against.
         */
        [[nodiscard]] bool Intersects(const BoundingCircle2D& circle) const;

        /**
         * @brief Tests if this line intersects with a capsule.
         * @param capsule The capsule to test against.
         */
        [[nodiscard]] bool Intersects(const BoundingCapsule2D& capsule) const;

        /**
         * @brief Deconstructs this line into its component values.
         * @param normal Receives the unit normal vector.
         * @param distance Receives the signed perpendicular distance from the origin.
         */
        void Deconstruct(Vector2& normal, float& distance) const;

        [[nodiscard]] bool Equals(const Line2D& other) const;
        [[nodiscard]] int GetHashCode() const;
        [[nodiscard]] std::string ToString() const;

        friend bool operator==(const Line2D& left, const Line2D& right) { return left.Equals(right); }
        friend bool operator!=(const Line2D& left, const Line2D& right) { return !left.Equals(right); }
    };
}
