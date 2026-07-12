// SPDX-License-Identifier: MIT
// Copyright (c) Robert Vokac and contributors
// Portions based on MonoGame.Extended (MIT License, Copyright (c) Craftwork Games)
//
// Ported from MonoGame.Extended's LineSegment2D.cs. Unlike Line2D (where several Intersects
// overloads were self-contained), essentially every distance/intersection query here delegates to
// a real Collision2D algorithm, not a trivial constant:
//   - DistanceSquaredToPoint/DistanceToPoint -> Collision2D.DistanceSquaredPointSegment
//   - DistanceSquaredToSegment/DistanceToSegment -> Collision2D.DistanceSquaredSegmentSegment
//   - Intersects(Line2D, ...) -> Line2D::Intersects(LineSegment2D, ...), itself deferred in
//     Line2D.hpp for the same reason
//   - Intersects(Ray2D, ...) -> Ray2D::Intersects(LineSegment2D, ...), deferred in Ray2D.hpp
//   - Intersects(LineSegment2D, ...) -> Collision2D.SolveParametricIntersection2D
//   - Intersects(BoundingBox2D/BoundingCircle2D/BoundingCapsule2D/OrientedBoundingBox2D/
//     BoundingPolygon2D, ...) -> Collision2D.ClipLineToAabb / RayCircleIntersectionInterval /
//     RayCapsuleIntersectionInterval / ClipLineToConvexPolygon / ClipInterval, plus
//     BoundingPolygon2D's own Contains(...) (itself Collision2D-dependent, deferred in
//     BoundingPolygon2D.hpp)
// All of the above are deferred to Phase 2 ("Collisions 2D"), not this phase.
//
// IMPORTANT correction to BoundingCapsule2D.hpp's deferred-method comment (written when
// BoundingCapsule2D was ported, before this file existed): it assumed CreateFromSegment/
// CreateMerged would become portable once LineSegment2D landed. That is NOT the case --
// DistanceSquaredToPoint itself needs Collision2D, so those two BoundingCapsule2D methods remain
// blocked until Collision2D is ported, not just until LineSegment2D exists. See NEXT.md.
//
// What IS fully self-contained and ported here: fields, Direction/Midpoint/Length/LengthSquared,
// the constructor, GetBounds(), GetPoint(...), ClosestPoint(...) (a real algorithm, but one that
// only uses Vector2 arithmetic, not Collision2D), Deconstruct, Equals/GetHashCode/ToString/
// operators (Equals here is exact field equality, unlike Line2D's epsilon-based Equals -- matches
// upstream, which does not use Collision2D.Epsilon in this type's Equals).
#pragma once

#include "Microsoft/Xna/Framework/Vector2.hpp"

#include <string>

namespace CNA::Extended
{
    using Microsoft::Xna::Framework::Vector2;

    class BoundingBox2D;

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
